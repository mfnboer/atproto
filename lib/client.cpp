// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "client.h"
#include "xjson.h"
#include "lexicon/com_atproto_identity.h"
#include "lexicon/lexicon.h"
#include <QTimer>
#include <QUrl>
#include <QUuid>

namespace ATProto
{

#define SERVICE_KEY_ATPROTO_LABELER QStringLiteral("atproto_labeler")
#define SERVICE_KEY_BSKY_CHAT QStringLiteral("bsky_chat")
#define SERVICE_KEY_BSKY_FEEDGEN QStringLiteral("bsky_fg")

#define SERVICE_DID_BSKY_CHAT QStringLiteral("did:web:api.bsky.chat")
#define SERVICE_DID_BSKY_VIDEO QStringLiteral("did:web:video.bsky.app")

constexpr char const* ERROR_INVALID_JSON = "InvalidJson";
constexpr char const* ERROR_INVALID_SESSION = "InvalidSession";

static QString boolValue(bool value)
{
    return value ? QStringLiteral("true") : QStringLiteral("false");
}

static void addOptionalIntParam(Xrpc::Client::Params& params, const QString& name, std::optional<int> value, int min, int max)
{
    if (value)
    {
        if (*value < min || * value > max)
            throw InvalidRequest(QString("Invalid %1 value %2").arg(name, *value));

        params.append({name, QString::number(*value)});
    }
}

static void addOptionalStringParam(Xrpc::Client::Params& params, const QString& name,
                                   const std::optional<QString>& value)
{
    if (value)
        params.append({name, *value});
}

static void addOptionalDateTimeParam(Xrpc::Client::Params& params, const QString& name,
                                 const std::optional<QDateTime>& value)
{
    if (value)
        params.append({name, value->toString(Qt::ISODateWithMs)});
}

static void addOptionalBoolParam(Xrpc::Client::Params& params, const QString& name,
                                 const std::optional<bool>& value)
{
    if (value)
        params.append({name, boolValue(*value)});
}

// TODO: is there a better way to identify errors?
bool Client::isListNotFoundError(const QString& error)
{
    // Currently INVALID_REQUEST is returned when a list does not exist. But I have
    // seen errors getting changed before. Test for NOT_FOUND as a precaution.
    return error == ATProtoErrorMsg::INVALID_REQUEST || error == ATProtoErrorMsg::NOT_FOUND;
}

Client::Client(std::unique_ptr<Xrpc::Client>&& xrpc) :
    mXrpc(std::move(xrpc))
{}

bool Client::setLabelerDids(const std::unordered_set<QString>& dids)
{
    if (dids.size() > MAX_LABELERS)
    {
        qDebug() << "Too many labelers:" << dids.size();
        return false;
    }

    if (mLabelerDids == dids)
    {
        qDebug() << "No change";
        return false;
    }

    mLabelerDids = dids;
    setAcceptLabelersHeaderValue();
    return true;
}

bool Client::addLabelerDid(const QString& did)
{
    if (mLabelerDids.size() >= MAX_LABELERS)
    {
        qWarning() << "Maximum labelers reached:" << mLabelerDids.size();
        return false;
    }

    mLabelerDids.insert(did);
    setAcceptLabelersHeaderValue();
    return true;
}

void Client::removeLabelerDid(const QString& did)
{
    mLabelerDids.erase(did);
    setAcceptLabelersHeaderValue();
}

void Client::updateTokens(const QString& accessJwt, const QString& refreshJwt)
{
    if (mSession)
    {
        qDebug() << "Update tokens";
        mSession->mAccessJwt = accessJwt;
        mSession->mRefreshJwt = refreshJwt;
    }
    else
    {
        qWarning() << "No session";
    }
}

void Client::createSession(const QString& user, const QString& pwd,
                           const std::optional<QString>& authFactorToken,
                           const SuccessCb& successCb, const ErrorCb& errorCb)
{
    if (user.startsWith("did:"))
    {
        qDebug() << "User is did:" << user;

        mXrpc->setPDSFromDid(user,
            [this, user, pwd, authFactorToken, successCb, errorCb]{
                createSessionContinue(user, pwd, authFactorToken, successCb, errorCb);
            },
            [errorCb](const QString& error){
                if (errorCb)
                    errorCb("PdsNotFound", error);
            });
    }
    else
    {
        qDebug() << "User is handle:" << user;

        mXrpc->setPDSFromHandle(user,
            [this, user, pwd, authFactorToken, successCb, errorCb]{
                createSessionContinue(user, pwd, authFactorToken, successCb, errorCb);
            },
            [errorCb](const QString& error){
                if (errorCb)
                    errorCb("PdsNotFound", error);
            });
    }
}

void Client::createSessionContinue(const QString& user, const QString& pwd,
                                   const std::optional<QString>& authFactorToken,
                                   const SuccessCb& successCb, const ErrorCb& errorCb)
{
    mSession = nullptr;
    QJsonObject root;
    root.insert("identifier", user);
    root.insert("password", pwd);
    XJsonObject::insertOptionalJsonValue(root, "authFactorToken", authFactorToken);
    QJsonDocument json(root);

    mXrpc->post("com.atproto.server.createSession", json, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qInfo() << "Session created:" << reply;
            try {
                mSession = ComATProtoServer::Session::fromJson(reply);
                mXrpc->setPDSFromSession(*mSession);

                if (successCb)
                    successCb();
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb));
}

void Client::deleteSession(const SuccessCb& successCb, const ErrorCb& errorCb)
{
    if (!mSession)
    {
        qWarning() << "There is no session";
        return;
    }

    mXrpc->post("com.atproto.server.deleteSession", {}, {},
        [this, successCb](const QJsonDocument& reply){
            qDebug() << "Delete session reply:" << reply;
            clearSession();

            if (successCb)
                successCb();
        },
        failure(errorCb),
        refreshToken());
}

void Client::resumeSession(const ComATProtoServer::Session& session,
                           const SuccessCb& successCb, const ErrorCb& errorCb)
{
    mXrpc->setPDSFromDid(session.mDid,
        [this, session, successCb, errorCb]{
            resumeSessionContinue(session, successCb, errorCb);
        },
        [errorCb](const QString& error){
            if (errorCb)
                errorCb("PdsNotFound", error);
        });
}

void Client::resumeSessionContinue(const ComATProtoServer::Session& session,
    const SuccessCb& successCb, const ErrorCb& errorCb)
{
    mXrpc->get("com.atproto.server.getSession", {}, {},
        [this, session, successCb, errorCb](const QJsonDocument& reply){
            qInfo() << "Got session:" << reply;
            try {
                auto resumed = ComATProtoServer::GetSessionOutput::fromJson(reply);
                if (resumed->mDid == session.mDid)
                {
                    qInfo() << "Session resumed";
                    mSession = std::make_shared<ComATProtoServer::Session>(session);
                    mSession->mHandle = resumed->mHandle;
                    mSession->mEmail = resumed->mEmail;
                    mSession->mEmailConfirmed = resumed->mEmailConfirmed;
                    mSession->mEmailAuthFactor = resumed->mEmailAuthFactor;
                    mSession->mDidDoc = resumed->mDidDoc;
                    mXrpc->setPDSFromSession(*mSession);

                    if (successCb)
                        successCb();
                }
                else
                {
                    const auto msg = QString("Session did(%1) does not match resumed did(%2)").arg(
                        session.mDid, resumed->mDid);
                    qWarning() << msg;

                    if (errorCb)
                        errorCb(ERROR_INVALID_SESSION, msg);
                }
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        session.mAccessJwt);
}

void Client::refreshSession(const SuccessCb& successCb, const ErrorCb& errorCb)
{
    mXrpc->post("com.atproto.server.refreshSession", {}, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "Refresh session reply:" << reply;
            try {
                auto refreshed = ComATProtoServer::Session::fromJson(reply);
                if (refreshed->mDid == mSession->mDid)
                {
                    qDebug() << "Session refreshed";
                    mSession->mAccessJwt = refreshed->mAccessJwt;
                    mSession->mRefreshJwt = refreshed->mRefreshJwt;
                    mSession->mHandle = refreshed->mHandle;
                    mSession->mDidDoc = refreshed->mDidDoc;
                    mXrpc->setPDSFromSession(*mSession);

                    if (successCb)
                        successCb();
                }
                else
                {
                    const auto msg = QString("Session did(%1) does not match refreshed did(%2)").arg(
                        mSession->mDid, refreshed->mDid);
                    qWarning() << msg;

                    if (errorCb)
                        errorCb(ERROR_INVALID_SESSION, msg);
                }
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        refreshToken());
}

void Client::getAccountInviteCodes(const GetAccountInviteCodesSuccessCb& successCb, const ErrorCb& errorCb)
{
    mXrpc->get("com.atproto.server.getAccountInviteCodes", {}, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getAccountInviteCodes reply:" << reply;
            try {
                auto output = ComATProtoServer::GetAccountInviteCodesOutput::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getServiceAuth(const QString& aud, const std::optional<QDateTime>& expiry, const std::optional<QString>& lexiconMethod,
                    const GetServiceAuthSuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Get serviceAuth:" << aud;
    Xrpc::Client::Params params{{"aud", aud}};

    if (expiry)
        params.append({"exp", QString::number(expiry->toSecsSinceEpoch())});

    addOptionalStringParam(params, "lxm", lexiconMethod);

    mXrpc->get("com.atproto.server.getServiceAuth", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getServiceAuth reply:" << reply;
            try {
                auto output = ComATProtoServer::GetServiceAuthOutput::fromJson(reply.object());

                if (successCb)
                    successCb(output);
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::resolveHandle(const QString& handle,
                   const ResolveHandleSuccessCb& successCb, const ErrorCb& errorCb)
{
    mXrpc->get("com.atproto.identity.resolveHandle", {{"handle", handle}}, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "resolveHandle:" << reply;
            try {
                auto output = ComATProtoIdentity::ResolveHandleOutput::fromJson(reply.object());
                if (successCb)
                    successCb(output->mDid);
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getProfile(const QString& user, const GetProfileSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params httpHeaders;
    addAcceptLabelersHeader(httpHeaders);

    mXrpc->get("app.bsky.actor.getProfile", {{"actor", user}}, httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getProfile:" << reply;
            try {
                auto profile = AppBskyActor::ProfileViewDetailed::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(profile));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getProfiles(const std::vector<QString>& users, const GetProfilesSuccessCb& successCb, const ErrorCb& errorCb)
{
    Q_ASSERT(users.size() > 0);
    Q_ASSERT(users.size() <= MAX_IDS_GET_PROFILES);
    Xrpc::Client::Params params;

    for (const auto& user : users)
        params.append({"actors", user});

    Xrpc::Client::Params httpHeaders;
    addAcceptLabelersHeader(httpHeaders);

    mXrpc->get("app.bsky.actor.getProfiles", params, httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getProfiles:" << reply;
            try {
                AppBskyActor::ProfileViewDetailedList profiles;
                AppBskyActor::getProfileViewDetailedList(profiles, reply.object());

                if (successCb)
                    successCb(std::move(profiles));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getPreferences(const UserPrefsSuccessCb& successCb, const ErrorCb& errorCb)
{
    mXrpc->get("app.bsky.actor.getPreferences", {}, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getPreferences:" << reply;
            try {
                auto prefs = AppBskyActor::GetPreferencesOutput::fromJson(reply.object());

                if (successCb)
                    successCb(UserPreferences(prefs->mPreferences));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::putPreferences(const UserPreferences& userPrefs,
                            const SuccessCb& successCb, const ErrorCb& errorCb)
{
    AppBskyActor::GetPreferencesOutput prefs;
    prefs.mPreferences = userPrefs.toPreferenceList();
    auto json = prefs.toJson();

    mXrpc->post("app.bsky.actor.putPreferences", QJsonDocument(json), {},
        [successCb](const QJsonDocument& reply){
            qDebug() << "putPreferences:" << reply;
            if (successCb)
                successCb();
        },
        failure(errorCb),
        authToken());
}

void Client::searchActors(const QString& q, std::optional<int> limit, const std::optional<QString>& cursor,
                  const SearchActorsSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"q", q}};
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    Xrpc::Client::Params httpHeaders;
    addAcceptLabelersHeader(httpHeaders);

    mXrpc->get("app.bsky.actor.searchActors", params, httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "searchActors:" << reply;
            try {
                auto ouput = AppBskyActor::SearchActorsOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(ouput));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::searchActorsTypeahead(const QString& q, std::optional<int> limit,
                                   const SearchActorsTypeaheadSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"q", q}};
    addOptionalIntParam(params, "limit", limit, 1, 100);

    mXrpc->get("app.bsky.actor.searchActorsTypeahead", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "searchActorsTypeahead:" << reply;
            try {
                auto ouput = AppBskyActor::SearchActorsTypeaheadOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(ouput));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getSuggestions(std::optional<int> limit, const std::optional<QString>& cursor,
                            const QStringList& acceptLanguages,
                            const GetSuggestionsSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params;
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    Xrpc::Client::Params httpHeaders;
    addAcceptLanguageHeader(httpHeaders, acceptLanguages);
    addAcceptLabelersHeader(httpHeaders);

    mXrpc->get("app.bsky.actor.getSuggestions", params, httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getSuggestions: ok";
            try {
                auto ouput = AppBskyActor::GetSuggestionsOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(ouput));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getSuggestedFollows(const QString& user, const QStringList& acceptLanguages,
                                 const GetSuggestedFollowsSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"actor", user}};
    Xrpc::Client::Params httpHeaders;
    addAcceptLanguageHeader(httpHeaders, acceptLanguages);
    addAcceptLabelersHeader(httpHeaders);

    mXrpc->get("app.bsky.graph.getSuggestedFollowsByActor", params, httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getSuggestedFOllows:" << reply;
            try {
                auto ouput = AppBskyActor::GetSuggestedFollowsByActor::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(ouput));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getServices(const std::vector<QString>& dids, bool detailed,
                         const GetServicesSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"detailed", boolValue(detailed)}};

    for (const auto& did : dids)
        params.append({"dids", did});

    mXrpc->get("app.bsky.labeler.getServices", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getServices: success";
            try {
                auto output = AppBskyLabeler::GetServicesOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getAuthorFeed(const QString& user, std::optional<int> limit, const std::optional<QString>& cursor,
                           const std::optional<QString> filter, std::optional<bool> includePins,
                           const GetAuthorFeedSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"actor", user}};
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);
    addOptionalStringParam(params, "filter", filter);
    addOptionalBoolParam(params, "includePins", includePins);

    Xrpc::Client::Params httpHeaders;
    addAcceptLabelersHeader(httpHeaders);

    mXrpc->get("app.bsky.feed.getAuthorFeed", params, httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getAuthorFeed: ok";
            try {
                auto feed = AppBskyFeed::OutputFeed::fromJson(reply);
                if (successCb)
                    successCb(std::move(feed));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getActorLikes(const QString& user, std::optional<int> limit, const std::optional<QString>& cursor,
                           const GetActorLikesSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"actor", user}};
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    mXrpc->get("app.bsky.feed.getActorLikes", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getActorLikes:" << reply;
            try {
                auto feed = AppBskyFeed::OutputFeed::fromJson(reply);
                if (successCb)
                    successCb(std::move(feed));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getTimeline(std::optional<int> limit, const std::optional<QString>& cursor,
                         const GetTimelineSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params;
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    Xrpc::Client::Params httpHeaders;
    addAcceptLabelersHeader(httpHeaders);

    mXrpc->get("app.bsky.feed.getTimeline", params, httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getTimeline succeeded";
            try {
                auto feed = AppBskyFeed::OutputFeed::fromJson(reply);
                if (successCb)
                    successCb(std::move(feed));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getFeed(const QString& feed, std::optional<int> limit, const std::optional<QString>& cursor,
                     const QStringList& acceptLanguages,
                     const GetFeedSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"feed", feed}};
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    Xrpc::Client::Params httpHeaders;
    addAcceptLanguageHeader(httpHeaders, acceptLanguages);
    addAcceptLabelersHeader(httpHeaders);

    mXrpc->get("app.bsky.feed.getFeed", params, httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getFeed: ok";
            try {
                auto feed = AppBskyFeed::OutputFeed::fromJson(reply);
                if (successCb)
                    successCb(std::move(feed));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getListFeed(const QString& list, std::optional<int> limit, const std::optional<QString>& cursor,
                         const QStringList& acceptLanguages,
                         const GetFeedSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"list", list}};
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    Xrpc::Client::Params httpHeaders;
    addAcceptLanguageHeader(httpHeaders, acceptLanguages);
    addAcceptLabelersHeader(httpHeaders);

    mXrpc->get("app.bsky.feed.getListFeed", params, httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getListFeed: ok";
            try {
                auto feed = AppBskyFeed::OutputFeed::fromJson(reply);
                if (successCb)
                    successCb(std::move(feed));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getFeedGenerator(const QString& feed,
                      const GetFeedGeneratorSuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Get feed generator:" << feed;
    Xrpc::Client::Params params{{ "feed", feed }};

    mXrpc->get("app.bsky.feed.getFeedGenerator", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getFeedGenerator:" << reply;
            try {
                auto feed = AppBskyFeed::GetFeedGeneratorOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(feed));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getFeedGenerators(const std::vector<QString>& feeds,
                       const GetFeedGeneratorsSuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Get feed generators";
    Xrpc::Client::Params params;

    for (const auto& f : feeds)
    {
        qDebug() << f;
        params.append({"feeds", f});
    }

    mXrpc->get("app.bsky.feed.getFeedGenerators", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getFeedGenerators:" << reply;
            try {
                auto feed = AppBskyFeed::GetFeedGeneratorsOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(feed));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getActorFeeds(const QString& user, std::optional<int> limit, const std::optional<QString>& cursor,
                           const GetActorFeedsSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"actor", user}};
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    mXrpc->get("app.bsky.feed.getActorFeeds", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getActorFeeds:" << reply;
            try {
                auto output = AppBskyFeed::GetActorFeedsOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getPostThread(const QString& uri, std::optional<int> depth, std::optional<int> parentHeight,
                           const GetPostThreadSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"uri", uri}};
    addOptionalIntParam(params, "depth", depth, 0, 1000);
    addOptionalIntParam(params, "parentHeight", parentHeight, 0, 1000);

    Xrpc::Client::Params httpHeaders;
    addAcceptLabelersHeader(httpHeaders);

    mXrpc->get("app.bsky.feed.getPostThread", params, httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getPostThread OK";
            try {
                auto thread = AppBskyFeed::PostThread::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(thread));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getPosts(const std::vector<QString>& uris,
                      const GetPostsSuccessCb& successCb, const ErrorCb& errorCb)
{
    Q_ASSERT(uris.size() > 0);
    Q_ASSERT(uris.size() <= MAX_URIS_GET_POSTS);
    Xrpc::Client::Params params;

    for (const auto& uri : uris)
        params.append({"uris", uri});

    Xrpc::Client::Params httpHeaders;
    addAcceptLabelersHeader(httpHeaders);

    mXrpc->get("app.bsky.feed.getPosts", params, httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getPosts:" << reply;
            try {
                AppBskyFeed::PostViewList posts;
                AppBskyFeed::getPostViewList(posts, reply.object());

                if (successCb)
                    successCb(std::move(posts));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getQuotes(const QString& uri, const std::optional<QString>& cid, std::optional<int> limit,
                       const std::optional<QString>& cursor, const GetQuotesSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"uri", uri}};
    addOptionalStringParam(params, "cid", cid);
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    Xrpc::Client::Params httpHeaders;
    addAcceptLabelersHeader(httpHeaders);

    mXrpc->get("app.bsky.feed.getQuotes", params, httpHeaders,
               [this, successCb, errorCb](const QJsonDocument& reply){
                   qDebug() << "getQuotes:" << reply;
                   try {
                       auto output = AppBskyFeed::GetQuotesOutput::fromJson(reply.object());

                       if (successCb)
                           successCb(std::move(output));
                   } catch (InvalidJsonException& e) {
                       invalidJsonError(e, errorCb);
                   }
               },
               failure(errorCb),
               authToken());
}

void Client::searchPosts(const QString& q, std::optional<int> limit, const std::optional<QString>& cursor,
                         const std::optional<QString>& sort, const std::optional<QString>& author,
                         const std::optional<QString>& mentions, const std::optional<QDateTime>& since,
                         const std::optional<QDateTime>& until, const std::optional<QString>& lang,
                         const SearchPostsSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"q", q}};
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);
    addOptionalStringParam(params, "sort", sort);
    addOptionalStringParam(params, "author", author);
    addOptionalStringParam(params, "mentions", mentions);
    addOptionalDateTimeParam(params, "since", since);
    addOptionalDateTimeParam(params, "until", until);
    addOptionalStringParam(params, "lang", lang); // The spec says "langs" is an array???

    Xrpc::Client::Params httpHeaders;
    addAcceptLabelersHeader(httpHeaders);

    mXrpc->get("app.bsky.feed.searchPosts", params, httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "searchPosts:" << reply;
            try {
                auto output = AppBskyFeed::SearchPostsOutput::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getLikes(const QString& uri, std::optional<int> limit, const std::optional<QString>& cursor,
              const GetLikesSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"uri", uri}};
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    mXrpc->get("app.bsky.feed.getLikes", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getLikes:" << reply;
            try {
                auto likes = AppBskyFeed::GetLikesOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(likes));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getRepostedBy(const QString& uri, std::optional<int> limit, const std::optional<QString>& cursor,
                   const GetRepostedBySuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"uri", uri}};
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    mXrpc->get("app.bsky.feed.getRepostedBy", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getRepostedBy:" << reply;
            try {
                auto repostedBy = AppBskyFeed::GetRepostedByOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(repostedBy));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::sendInteractions(const AppBskyFeed::InteractionList& interactions, const QString& feedDid,
                              const SuccessCb& successCb, const ErrorCb& errorCb)
{
    const auto jsonArray = XJsonObject::toJsonArray<AppBskyFeed::Interaction>(interactions);

    Xrpc::Client::Params httpHeaders;
    addAtprotoProxyHeader(httpHeaders, feedDid, SERVICE_KEY_BSKY_FEEDGEN);

    mXrpc->post("app.bsky.feed.sendInteractions", QJsonDocument(jsonArray), httpHeaders,
        [successCb](const QJsonDocument& reply){
            qDebug() << "sendInteractions:" << reply;
            if (successCb)
                successCb();
        },
        failure(errorCb),
        authToken());
}

void Client::getFollows(const QString& actor, std::optional<int> limit, const std::optional<QString>& cursor,
                        const GetFollowsSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"actor", actor}};
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    mXrpc->get("app.bsky.graph.getFollows", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getFollows:" << reply;
            try {
                auto follows = AppBskyGraph::GetFollowsOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(follows));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getFollowers(const QString& actor, std::optional<int> limit, const std::optional<QString>& cursor,
                          const GetFollowersSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"actor", actor}};
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    mXrpc->get("app.bsky.graph.getFollowers", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getFollowers:" << reply;
            try {
                auto follows = AppBskyGraph::GetFollowersOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(follows));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getKnownFollowers(const QString& actor, std::optional<int> limit, const std::optional<QString>& cursor,
                               const GetFollowersSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"actor", actor}};
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    mXrpc->get("app.bsky.graph.getKnownFollowers", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getKnownFollowers:" << reply;
            try {
                auto follows = AppBskyGraph::GetFollowersOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(follows));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getBlocks(std::optional<int> limit, const std::optional<QString>& cursor,
                       const GetBlocksSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params;
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    mXrpc->get("app.bsky.graph.getBlocks", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getBlocks:" << reply;
            try {
                auto blocks = AppBskyGraph::GetBlocksOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(blocks));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getMutes(std::optional<int> limit, const std::optional<QString>& cursor,
                      const GetMutesSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params;
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    mXrpc->get("app.bsky.graph.getMutes", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getMutes:" << reply;
            try {
                auto mutes = AppBskyGraph::GetMutesOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(mutes));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::muteActor(const QString& actor, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject jsonObj;
    jsonObj.insert("actor", actor);
    QJsonDocument json(jsonObj);

    mXrpc->post("app.bsky.graph.muteActor", json, {},
        [successCb](const QJsonDocument& reply){
            qDebug() << "muteActor:" << reply;
            if (successCb)
                successCb();
        },
        failure(errorCb),
        authToken());
}

void Client::unmuteActor(const QString& actor, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject jsonObj;
    jsonObj.insert("actor", actor);
    QJsonDocument json(jsonObj);

    mXrpc->post("app.bsky.graph.unmuteActor", json, {},
        [successCb](const QJsonDocument& reply){
            qDebug() << "unmuteActor:" << reply;
            if (successCb)
                successCb();
        },
        failure(errorCb),
        authToken());
}

void Client::muteThread(const QString& root, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject jsonObj;
    jsonObj.insert("root", root);
    QJsonDocument json(jsonObj);

    mXrpc->post("app.bsky.graph.muteThread", json, {},
        [successCb](const QJsonDocument& reply){
            qDebug() << "muteActor:" << reply;
            if (successCb)
                successCb();
        },
        failure(errorCb),
        authToken());
}

void Client::unmuteThread(const QString& root, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject jsonObj;
    jsonObj.insert("root", root);
    QJsonDocument json(jsonObj);

    mXrpc->post("app.bsky.graph.unmuteThread", json, {},
        [successCb](const QJsonDocument& reply){
            qDebug() << "muteActor:" << reply;
            if (successCb)
                successCb();
        },
        failure(errorCb),
        authToken());
}

void Client::getList(const QString& listUri, std::optional<int> limit, const std::optional<QString>& cursor,
                     const GetListSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"list", listUri}};
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    mXrpc->get("app.bsky.graph.getList", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getList:" << reply;
            try {
                auto output = AppBskyGraph::GetListOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getLists(const QString& actor, std::optional<int> limit, const std::optional<QString>& cursor,
                      const GetListsSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"actor", actor}};
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    mXrpc->get("app.bsky.graph.getLists", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getLists:" << reply;
            try {
                auto output = AppBskyGraph::GetListsOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getListBlocks(std::optional<int> limit, const std::optional<QString>& cursor,
                           const GetListsSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params;
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    mXrpc->get("app.bsky.graph.getListBlocks", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getListBlocks:" << reply;
            try {
                auto output = AppBskyGraph::GetListsOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getListMutes(std::optional<int> limit, const std::optional<QString>& cursor,
                          const GetListsSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params;
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    mXrpc->get("app.bsky.graph.getListMutes", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getListMutes:" << reply;
            try {
                auto output = AppBskyGraph::GetListsOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getActorStarterPacks(const QString& actor, std::optional<int> limit, const std::optional<QString>& cursor,
                                  const GetStarterPacksSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"actor", actor}};
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    Xrpc::Client::Params httpHeaders;
    addAcceptLabelersHeader(httpHeaders);

    mXrpc->get("app.bsky.graph.getActorStarterPacks", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getActorStarterPacks:" << reply;
            try {
                auto output = AppBskyGraph::GetStarterPacksOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getStarterPacks(const std::vector<QString>& uris,
                             const GetStarterPacksSuccessCb& successCb, const ErrorCb& errorCb)
{
    Q_ASSERT(uris.size() > 0);
    Q_ASSERT(uris.size() <= MAX_URIS_GET_STARTER_PACKS);
    Xrpc::Client::Params params;

    for (const auto& uri : uris)
        params.append({"uris", uri});

    Xrpc::Client::Params httpHeaders;
    addAcceptLabelersHeader(httpHeaders);

    mXrpc->get("app.bsky.graph.getStarterPacks", params, httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getStarterPacks:" << reply;
            try {
                auto output = AppBskyGraph::GetStarterPacksOutput::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getStarterPack(const QString& starterPack, const GetStarterPackSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params httpHeaders;
    addAcceptLabelersHeader(httpHeaders);

    mXrpc->get("app.bsky.graph.getStarterPack", {{"starterPack", starterPack}}, httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getStarterPack:" << reply;
            try {
                auto output = AppBskyGraph::GetStarterPackOutput::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output->mStarterPack));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::muteActorList(const QString& listUri, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject jsonObj;
    jsonObj.insert("list", listUri);
    QJsonDocument json(jsonObj);

    mXrpc->post("app.bsky.graph.muteActorList", json, {},
        [successCb](const QJsonDocument& reply){
            qDebug() << "muteActorList:" << reply;
            if (successCb)
                successCb();
        },
        failure(errorCb),
        authToken());
}

void Client::unmuteActorList(const QString& listUri, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject jsonObj;
    jsonObj.insert("list", listUri);
    QJsonDocument json(jsonObj);

    mXrpc->post("app.bsky.graph.unmuteActorList", json, {},
        [successCb](const QJsonDocument& reply){
            qDebug() << "unmuteActorList:" << reply;
            if (successCb)
                successCb();
        },
        failure(errorCb),
        authToken());
}

void Client::getUnreadNotificationCount(const std::optional<QDateTime>& seenAt, std::optional<bool> priority,
                                        const UnreadCountSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params;
    addOptionalDateTimeParam(params, "seenAt", seenAt);
    addOptionalBoolParam(params, "priority", priority);

    mXrpc->get("app.bsky.notification.getUnreadCount", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getUnreadCount:" << reply;
            try {
                auto unreadCount = XJsonObject(reply.object()).getRequiredInt("count");
                if (successCb)
                    successCb(unreadCount);
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::updateNotificationSeen(const QDateTime& dateTime,
                                    const SuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonDocument json;
    QJsonObject paramsJson;
    paramsJson.insert("seenAt", dateTime.toString(Qt::ISODateWithMs));
    json.setObject(paramsJson);

    mXrpc->post("app.bsky.notification.updateSeen", json, {},
        [successCb](const QJsonDocument& reply){
            qDebug() << "Updated notification seen:" << reply;
            if (successCb)
                successCb();
        },
        failure(errorCb),
        authToken());
}

void Client::listNotifications(std::optional<int> limit, const std::optional<QString>& cursor,
                               const std::optional<QDateTime>& seenAt, std::optional<bool> priority,
                               const std::vector<AppBskyNotification::NotificationReason> reasons,
                               const NotificationsSuccessCb& successCb, const ErrorCb& errorCb,
                               bool updateSeen)
{
    Xrpc::Client::Params params;
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);
    addOptionalDateTimeParam(params, "seenAt", seenAt);
    addOptionalBoolParam(params, "priority", priority);

    for (const auto reason : reasons)
    {
        const auto reasonString = AppBskyNotification::notificationReasonToString(reason);

        if (!reasonString.isEmpty())
            params.append({"reasons", reasonString});
    }

    const auto now = QDateTime::currentDateTimeUtc();

    mXrpc->get("app.bsky.notification.listNotifications", params, {},
        [this, now, successCb, errorCb, updateSeen](const QJsonDocument& reply){
            try {
                qDebug() << "List notifications:" << reply;
                auto output = AppBskyNotification::ListNotificationsOutput::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output));

                if (updateSeen)
                    updateNotificationSeen(now, {}, {});
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::putNotificationPreferences(bool priority,
                                        const SuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonDocument json;
    QJsonObject paramsJson;
    paramsJson.insert("priority", priority);
    json.setObject(paramsJson);

    mXrpc->post("app.bsky.notification.putPreferences", json, {},
        [successCb](const QJsonDocument& reply){
            qDebug() << "Put notification preferences:" << reply;
            if (successCb)
                successCb();
        },
        failure(errorCb),
        authToken());
}

void Client::registerPushNotifications(const QString& serviceDid, const QString& token,
                               const QString& platform, const QString& appId,
                               const SuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonDocument json;
    QJsonObject paramsJson;
    paramsJson.insert("serviceDid", serviceDid);
    paramsJson.insert("token", token);
    paramsJson.insert("platform", platform);
    paramsJson.insert("appId", appId);
    json.setObject(paramsJson);

    qDebug() << json;

    mXrpc->post("app.bsky.notification.registerPush", json, {},
        [successCb](const QJsonDocument& reply){
            qDebug() << "registerPush succeeded:" << reply;
            if (successCb)
                successCb();
        },
        failure(errorCb),
        authToken());
}

void Client::getVideoJobStatus(const QString& jobId, const VideoJobStatusOutputCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"jobId", jobId}};

    mXrpc->get("app.bsky.video.getJobStatus", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            try {
                qDebug() << "Get video job status:" << reply;
                auto output = AppBskyVideo::JobStatusOutput::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getVideoUploadLimits(const GetVideoUploadLimitsCb& successCb, const ErrorCb& errorCb)
{
    getServiceAuth(SERVICE_DID_BSKY_VIDEO, {}, "app.bsky.video.getUploadLimits",
        [this, successCb, errorCb](auto output){
            getVideoUploadLimits(output->mToken, successCb, errorCb);
        },
        [errorCb](const QString& error, const QString& msg){
            if (errorCb)
                errorCb(error, msg);
        });
}

void Client::getVideoUploadLimits(const QString& serviceAuthToken, const GetVideoUploadLimitsCb& successCb, const ErrorCb& errorCb)
{
    mXrpc->get("app.bsky.video.getUploadLimits", {}, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            try {
                qDebug() << "Get video upload limits:" << reply;
                auto output = AppBskyVideo::GetUploadLimitsOutput::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        serviceAuthToken);
}

void Client::uploadVideo(QFile* blob, const VideoUploadOutputCb& successCb, const ErrorCb& errorCb)
{
    QUrl url(mXrpc->getPDS());
    QString aud = "did:web:" + url.host();
    QDateTime expiry = QDateTime::currentDateTimeUtc().addSecs(30 * 60);

    getServiceAuth(aud, expiry, "com.atproto.repo.uploadBlob",
        [this, blob, successCb, errorCb](auto output){
            uploadVideo(blob, output->mToken, successCb, errorCb);
        },
        [errorCb](const QString& error, const QString& msg){
            if (errorCb)
                errorCb(error, msg);
        });
}

void Client::uploadVideo(QFile* blob, const QString& serviceAuthToken, const VideoUploadOutputCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Upload video:" << blob->size();
    const QString name = QUuid::createUuid().toString(QUuid::WithoutBraces);
    const QString service = QString("app.bsky.video.uploadVideo?did=%1&name=%2.mp4").arg(QUrl::toPercentEncoding(mSession->mDid), name);
    qDebug() << "Service:" << service;

    mXrpc->post(service, blob, "video/mp4", {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "Upload video:" << reply;
            try {
                // According to the spec the reply should be an object with jobStatus field.
                // But it seems we get a JobStatus itself...
                auto ouput = AppBskyVideo::JobStatus::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(ouput));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        [this, successCb, errorCb](const QString& err, const QJsonDocument& reply){
            try {
                auto jobStatus = AppBskyVideo::JobStatus::fromJson(reply.object());

                // If the video is already uploaded, we get an error. The error gives
                // this job id, but not the blob. We can get the blob by getting the
                // job status for the job id.
                if (jobStatus->mState == AppBskyVideo::JobStatusState::JOB_STATE_COMPLETED &&
                    jobStatus->mError == ATProtoErrorMsg::ALREADY_EXISTS)
                {
                    qDebug() << "Video already exists:" << jobStatus->mJobId;

                    getVideoJobStatus(jobStatus->mJobId,
                        [successCb](const auto& jobStatusOutput){
                            if (successCb)
                                successCb(jobStatusOutput->mJobStatus);
                        },
                        [errorCb](const QString& error, const QString& message){
                            qWarning() << error << " - " << message;
                            if (errorCb)
                                errorCb(error, message);
                        });
                }
                else
                {
                    requestFailed(err, reply, errorCb);
                }
            } catch (InvalidJsonException& e) {
                qWarning() << e.msg();
                requestFailed(err, reply, errorCb);
            }
        },
        serviceAuthToken);
}

void Client::uploadBlob(const QByteArray& blob, const QString& mimeType,
                        const UploadBlobSuccessCb& successCb, const ErrorCb& errorCb)
{
    mXrpc->post("com.atproto.repo.uploadBlob", blob, mimeType, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "Upload blob:" << reply;
            try {
                auto ouput = ComATProtoRepo::UploadBlobOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(ouput->mBlob));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getBlob(const QString& did, const QString& cid,
                     const GetBlobSuccessCb& successCb, const ErrorCb& errorCb)
{
    mXrpc->setPDSFromDid(did,
        [this, did, cid, successCb, errorCb]{
            getBlobContinue(did, cid, successCb, errorCb);
        },
        [errorCb](const QString& error){
            if (errorCb)
                errorCb("PdsNotFound", error);
        });
}

void Client::getBlobContinue(const QString& did, const QString& cid,
                             const GetBlobSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"did", did}, {"cid", cid}};

    mXrpc->get("com.atproto.sync.getBlob", params, {},
        [successCb](const QByteArray& bytes, const QString& contentType){
            qDebug() <<"Got blob:" << bytes.size() << "bytes" << "content:" << contentType;

            if (successCb)
                successCb(bytes, contentType);
        },
        failure(errorCb));
}

void Client::getRecord(const QString& repo, const QString& collection,
                       const QString& rkey, const std::optional<QString>& cid,
                       const GetRecordSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"repo", repo}, {"collection", collection}, {"rkey", rkey}};
    addOptionalStringParam(params, "cid", cid);

    mXrpc->get("com.atproto.repo.getRecord", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() <<"Got record:" << reply;
            try {
                auto record = ComATProtoRepo::Record::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(record));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::listRecords(const QString& repo, const QString& collection,
                         std::optional<int> limit, const std::optional<QString>& cursor,
                         const ListRecordsSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"repo", repo}, {"collection", collection}};
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    mXrpc->get("com.atproto.repo.listRecords", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() <<"Got records:" << reply;
            try {
                auto record = ComATProtoRepo::ListRecordsOutput::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(record));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::createRecord(const QString& repo, const QString& collection, const QString& rkey,
                          const QJsonObject& record, bool validate,
                          const CreateRecordSuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject root;
    root.insert("repo", repo);
    root.insert("collection", collection);
    root.insert("record", record);
    root.insert("validate", validate);

    if (!rkey.isEmpty())
        root.insert("rkey", rkey);

    QJsonDocument json(root);

    qDebug() << "Create record:" << json;

    mXrpc->post("com.atproto.repo.createRecord", json, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "Created record:" << reply;
            try {
                auto recordRef = ComATProtoRepo::StrongRef::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(recordRef));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::putRecord(const QString& repo, const QString& collection, const QString& rkey,
                       const QJsonObject& record, bool validate,
                       const PutRecordSuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject root;
    root.insert("repo", repo);
    root.insert("collection", collection);
    root.insert("record", record);
    root.insert("rkey", rkey);
    root.insert("validate", validate);

    QJsonDocument json(root);

    qDebug() << "Put record:" << json;

    mXrpc->post("com.atproto.repo.putRecord", json, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "Put record ok:" << reply;
            try {
                auto recordRef = ComATProtoRepo::StrongRef::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(recordRef));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::deleteRecord(const QString& repo, const QString& collection, const QString& rkey,
                          const SuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonDocument jsonDoc;
    QJsonObject json;
    json.insert("repo", repo);
    json.insert("collection", collection);
    json.insert("rkey", rkey);
    jsonDoc.setObject(json);

    qDebug() << "Delete record:" << jsonDoc;

    mXrpc->post("com.atproto.repo.deleteRecord", jsonDoc, {},
        [successCb](const QJsonDocument& reply){
            qDebug() << "Deleted record:" << reply;
            if (successCb)
                successCb();
        },
        failure(errorCb),
        authToken());
}

void Client::applyWrites(const QString& repo, const ComATProtoRepo::ApplyWritesList& writes, bool validate,
                         const SuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject json;
    json.insert("repo", repo);
    json.insert("validate", validate);
    QJsonArray writesArray;

    for (const auto& write : writes)
    {
        QJsonObject writeJson;
        std::visit([&writeJson](auto&& x){ writeJson = x->toJson(); }, write);
        writesArray.push_back(writeJson);
    }

    json.insert("writes", writesArray);

    QJsonDocument jsonDoc;
    jsonDoc.setObject(json);
    qDebug() << "Apply writes:" << jsonDoc;

    mXrpc->post("com.atproto.repo.applyWrites", jsonDoc, {},
        [successCb](const QJsonDocument& reply){
            qDebug() << "Apply writes:" << reply;
            if (successCb)
                successCb();
        },
        failure(errorCb),
        authToken());
}

void Client::reportAuthor(const QString& did, ComATProtoModeration::ReasonType reasonType,
                          const QString& reason, const std::optional<QString>& labelerDid,
                          const SuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject json;
    json.insert("reasonType", ComATProtoModeration::reasonTypeToString(reasonType));

    if (!reason.isEmpty())
        json.insert("reason", reason);

    QJsonObject jsonRepoRef;
    jsonRepoRef.insert("$type", "com.atproto.admin.defs#repoRef");
    jsonRepoRef.insert("did", did);
    json.insert("subject", jsonRepoRef);

    QJsonDocument jsonDoc;
    jsonDoc.setObject(json);

    Xrpc::Client::Params httpHeaders;

    if (labelerDid)
        addAtprotoProxyHeader(httpHeaders, *labelerDid, SERVICE_KEY_ATPROTO_LABELER);

    qDebug() << "Report author:" << jsonDoc;
    qDebug() << "HTTP headers:" << httpHeaders;

    mXrpc->post("com.atproto.moderation.createReport", jsonDoc, httpHeaders,
        [successCb](const QJsonDocument& reply){
            qDebug() <<"Reported author:" << reply;
            if (successCb)
                successCb();
        },
        failure(errorCb),
        authToken());
}

void Client::reportPostOrFeed(const QString& uri, const QString& cid,
                              ComATProtoModeration::ReasonType reasonType,
                              const QString& reason, const std::optional<QString>& labelerDid,
                              const SuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject json;
    json.insert("reasonType", ComATProtoModeration::reasonTypeToString(reasonType));

    if (!reason.isEmpty())
        json.insert("reason", reason);

    ComATProtoRepo::StrongRef ref;
    ref.mUri = uri;
    ref.mCid = cid;
    json.insert("subject", ref.toJson());

    QJsonDocument jsonDoc;
    jsonDoc.setObject(json);

    Xrpc::Client::Params httpHeaders;

    if (labelerDid)
        addAtprotoProxyHeader(httpHeaders, *labelerDid, SERVICE_KEY_ATPROTO_LABELER);

    qDebug() << "Report post or feed:" << jsonDoc;
    qDebug() << "HTTP headers:" << httpHeaders;

    mXrpc->post("com.atproto.moderation.createReport", jsonDoc, httpHeaders,
        [successCb](const QJsonDocument& reply){
            qDebug() <<"Reported post or feed:" << reply;
            if (successCb)
                successCb();
        },
        failure(errorCb),
        authToken());
}

void Client::reportDirectMessage(const QString& did, const QString& convoId, const QString& messageId,
                                 ComATProtoModeration::ReasonType reasonType, const QString& reason,
                                 const SuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject json;
    json.insert("reasonType", ComATProtoModeration::reasonTypeToString(reasonType));

    if (!reason.isEmpty())
        json.insert("reason", reason);

    ChatBskyConvo::MessageRef ref;
    ref.mDid = did;
    ref.mConvoId = convoId;
    ref.mMessageId = messageId;
    json.insert("subject", ref.toJson());
    QJsonDocument jsonDoc(json);

    qDebug() << "Report direct message:" << jsonDoc;

    mXrpc->post("com.atproto.moderation.createReport", jsonDoc, {},
        [successCb](const QJsonDocument& reply){
            qDebug() <<"Reported direct message:" << reply;
            if (successCb)
                successCb();
        },
        failure(errorCb),
        authToken());
}

void Client::getPopularFeedGenerators(const std::optional<QString>& q, std::optional<int> limit,
                              const std::optional<QString>& cursor,
                              const GetPopularFeedGeneratorsSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params;
    addOptionalStringParam(params, "query", q);
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    mXrpc->get("app.bsky.unspecced.getPopularFeedGenerators", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getPopularFeedGenerators:" << reply;
            try {
                auto output = AppBskyUnspecced::GetPopularFeedGeneratorsOutput::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getTrendingTopics(const std::optional<QString>& viewer, std::optional<int> limit,
                       const GetTrendingTopicsSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params;
    addOptionalStringParam(params, "viewer", viewer);
    addOptionalIntParam(params, "limit", limit, 1, 25);

    mXrpc->get("app.bsky.unspecced.getTrendingTopics", params, {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getTrendingTopics:" << reply;
            try {
                auto output = AppBskyUnspecced::GetTrendingTopicsOutput::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::deleteMessageForSelf(const QString& convoId, const QString& messageId,
                          const DeleteMessageSuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject json;
    json.insert("convoId", convoId);
    json.insert("messageId", messageId);

    Xrpc::Client::Params httpHeaders;
    addAtprotoProxyHeader(httpHeaders, SERVICE_DID_BSKY_CHAT, SERVICE_KEY_BSKY_CHAT);

    mXrpc->post("chat.bsky.convo.deleteMessageForSelf", QJsonDocument(json), httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() <<"Delete message for self:" << reply;

            try {
                auto output = ChatBskyConvo::DeletedMessageView::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getConvo(const QString& convoId,
                      const ConvoSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"convoId", convoId}};

    Xrpc::Client::Params httpHeaders;
    addAcceptLabelersHeader(httpHeaders);
    addAtprotoProxyHeader(httpHeaders, SERVICE_DID_BSKY_CHAT, SERVICE_KEY_BSKY_CHAT);

    mXrpc->get("chat.bsky.convo.getConvo", params, httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getConvo:" << reply;
            try {
                auto output = ChatBskyConvo::ConvoOuput::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getConvoForMembers(const std::vector<QString>& members,
                                const ConvoSuccessCb& successCb, const ErrorCb& errorCb)
{
    Q_ASSERT(members.size() > 0);
    Q_ASSERT(members.size() <= MAX_CONVO_MEMBERS);
    Xrpc::Client::Params params;

    for (const auto& member : members)
        params.append({"members", member});

    Xrpc::Client::Params httpHeaders;
    addAcceptLabelersHeader(httpHeaders);
    addAtprotoProxyHeader(httpHeaders, SERVICE_DID_BSKY_CHAT, SERVICE_KEY_BSKY_CHAT);

    mXrpc->get("chat.bsky.convo.getConvoForMembers", params, httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getConvo:" << reply;
            try {
                auto output = ChatBskyConvo::ConvoOuput::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getConvoLog(const std::optional<QString>& cursor,
                         const ConvoLogSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params;
    addOptionalStringParam(params, "cursor", cursor);

    Xrpc::Client::Params httpHeaders;
    addAtprotoProxyHeader(httpHeaders, SERVICE_DID_BSKY_CHAT, SERVICE_KEY_BSKY_CHAT);

    mXrpc->get("chat.bsky.convo.getLog", params, httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getConvoLog:" << reply;
            try {
                auto output = ChatBskyConvo::LogOutput::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getMessages(const QString& convoId, std::optional<int> limit,
                         const std::optional<QString>& cursor,
                         const GetMessagesSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"convoId", convoId}};
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    Xrpc::Client::Params httpHeaders;
    addAtprotoProxyHeader(httpHeaders, SERVICE_DID_BSKY_CHAT, SERVICE_KEY_BSKY_CHAT);

    mXrpc->get("chat.bsky.convo.getMessages", params, httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getMessages:" << reply;
            try {
                auto output = ChatBskyConvo::GetMessagesOutput::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::leaveConvo(const QString& convoId,
                        const LeaveConvoSuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject json;
    json.insert("convoId", convoId);

    Xrpc::Client::Params httpHeaders;
    addAtprotoProxyHeader(httpHeaders, SERVICE_DID_BSKY_CHAT, SERVICE_KEY_BSKY_CHAT);

    mXrpc->post("chat.bsky.convo.leaveConvo", QJsonDocument(json), httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() <<"Leave convo:" << reply;

            try {
                auto output = ChatBskyConvo::LeaveConvoOutput::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::listConvos(std::optional<int> limit, const std::optional<QString>& cursor,
                        const ConvoListSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params;
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    Xrpc::Client::Params httpHeaders;
    addAcceptLabelersHeader(httpHeaders);
    addAtprotoProxyHeader(httpHeaders, SERVICE_DID_BSKY_CHAT, SERVICE_KEY_BSKY_CHAT);

    mXrpc->get("chat.bsky.convo.listConvos", params, httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "List convos: ok";
            try {
                auto output = ChatBskyConvo::ConvoListOutput::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::muteConvo(const QString& convoId,
                       const ConvoSuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject json;
    json.insert("convoId", convoId);

    Xrpc::Client::Params httpHeaders;
    addAtprotoProxyHeader(httpHeaders, SERVICE_DID_BSKY_CHAT, SERVICE_KEY_BSKY_CHAT);

    mXrpc->post("chat.bsky.convo.muteConvo", QJsonDocument(json), httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "Mute convo:" << reply;
            try {
                auto output = ChatBskyConvo::ConvoOuput::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::sendMessage(const QString& convoId, const ChatBskyConvo::MessageInput& message,
                         const MessageSuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject json;
    json.insert("convoId", convoId);
    json.insert("message", message.toJson());

    Xrpc::Client::Params httpHeaders;
    addAtprotoProxyHeader(httpHeaders, SERVICE_DID_BSKY_CHAT, SERVICE_KEY_BSKY_CHAT);

    mXrpc->post("chat.bsky.convo.sendMessage", QJsonDocument(json), httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() <<"Send message:" << reply;

            try {
                auto output = ChatBskyConvo::MessageView::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::unmuteConvo(const QString& convoId,
                         const ConvoSuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject json;
    json.insert("convoId", convoId);

    Xrpc::Client::Params httpHeaders;
    addAtprotoProxyHeader(httpHeaders, SERVICE_DID_BSKY_CHAT, SERVICE_KEY_BSKY_CHAT);

    mXrpc->post("chat.bsky.convo.unmuteConvo", QJsonDocument(json), httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "Unmute convo:" << reply;
            try {
                auto output = ChatBskyConvo::ConvoOuput::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::updateRead(const QString& convoId, const std::optional<QString>& messageId,
                        const ConvoSuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject json;
    json.insert("convoId", convoId);
    XJsonObject::insertOptionalJsonValue(json, "messageId", messageId);

    Xrpc::Client::Params httpHeaders;
    addAtprotoProxyHeader(httpHeaders, SERVICE_DID_BSKY_CHAT, SERVICE_KEY_BSKY_CHAT);

    mXrpc->post("chat.bsky.convo.updateRead", QJsonDocument(json), httpHeaders,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "Update read:" << reply;
            try {
                auto output = ChatBskyConvo::ConvoOuput::fromJson(reply.object());

                if (successCb)
                    successCb(std::move(output));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

const QString& Client::authToken() const
{
    static const QString NO_TOKEN;
    return mSession ? mSession->mAccessJwt : NO_TOKEN;
}

const QString& Client::refreshToken() const
{
    static const QString NO_TOKEN;
    return mSession ? mSession->mRefreshJwt : NO_TOKEN;
}

Xrpc::Client::ErrorCb Client::failure(const ErrorCb& cb)
{
    return [this, cb](const QString& err, const QJsonDocument& reply){
            requestFailed(err, reply, cb);
        };
}

void Client::invalidJsonError(InvalidJsonException& e, const ErrorCb& cb)
{
    qWarning() << e.msg();
    if (cb)
        cb(ERROR_INVALID_JSON, e.msg());
}

void Client::requestFailed(const QString& err, const QJsonDocument& json, const ErrorCb& errorCb)
{
    qInfo() << "Request failed:" << err;
    qInfo() << json;

    if (json.isNull())
    {
        if (errorCb)
            errorCb(err, err);

        return;
    }

    if (json.isEmpty())
    {
        if (errorCb)
            errorCb(ERROR_INVALID_JSON, err);

        return;
    }

    try {
        auto error = ATProtoError::fromJson(json);
        if (errorCb)
            errorCb(error->mError, error->mMessage);
    } catch (InvalidJsonException& e) {
        qWarning() << e.msg();
        if (errorCb)
            errorCb(ERROR_INVALID_JSON, err);
    }
}

void Client::setAcceptLabelersHeaderValue()
{
    mAcceptLabelersHeaderValue.clear();

    for (const auto& did : mLabelerDids)
    {
        if (!mAcceptLabelersHeaderValue.isEmpty())
            mAcceptLabelersHeaderValue.push_back(',');

        mAcceptLabelersHeaderValue.push_back(did);
    }

    qDebug() << "Labelers:" << mAcceptLabelersHeaderValue;
}

void Client::addAcceptLabelersHeader(Xrpc::Client::Params& httpHeaders) const
{
    if (!mAcceptLabelersHeaderValue.isEmpty())
        httpHeaders.push_back({"atproto-accept-labelers", mAcceptLabelersHeaderValue});
}

void Client::addAcceptLanguageHeader(Xrpc::Client::Params& httpHeaders, const QStringList& languages) const
{
    if (!languages.empty())
        httpHeaders.push_back({"Accept-Language", languages.join(',')});
}

void Client::addAtprotoProxyHeader(Xrpc::Client::Params& httpHeaders, const QString& did, const QString& serviceKey) const
{
    const QString value = QString("%1#%2").arg(did, serviceKey);
    qDebug() << "Proxy:" << value;
    httpHeaders.push_back({"atproto-proxy", value});
}

}
