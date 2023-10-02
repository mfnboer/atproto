// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "client.h"
#include "at_uri.h"
#include "tlds.h"
#include "xjson.h"
#include "lexicon/com_atproto_identity.h"
#include "lexicon/lexicon.h"
#include <QRegularExpression>
#include <QTimer>
#include <QUrl>

namespace ATProto
{

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


Client::Client(std::unique_ptr<Xrpc::Client>&& xrpc) :
    mXrpc(std::move(xrpc))
{}

void Client::createSession(const QString& user, const QString& pwd,
                           const SuccessCb& successCb, const ErrorCb& errorCb)
{
    mSession = nullptr;
    QJsonObject root;
    root.insert("identifier", user);
    root.insert("password", pwd);
    QJsonDocument json(root);

    mXrpc->post("com.atproto.server.createSession", json,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qInfo() << "Session created:" << reply;
            try {
                mSession = std::move(ComATProtoServer::Session::fromJson(reply));
                if (successCb)
                    successCb();
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb));
}

void Client::resumeSession(const ComATProtoServer::Session& session,
                           const SuccessCb& successCb, const ErrorCb& errorCb)
{
    mXrpc->get("com.atproto.server.getSession", {},
        [this, session, successCb, errorCb](const QJsonDocument& reply){
            qInfo() << "Got session:" << reply;
            try {
                auto resumed = ComATProtoServer::GetSessionOutput::fromJson(reply);
                if (resumed->mDid == session.mDid)
                {
                    qInfo() << "Session resumed";
                    mSession = std::make_unique<ComATProtoServer::Session>(session);
                    mSession->mHandle = resumed->mHandle;
                    mSession->mEmail = resumed->mEmail;

                    if (successCb)
                        successCb();
                }
                else
                {
                    const auto msg = QString("Session did(%1) does not match resumed did(%2)").arg(
                        session.mDid, resumed->mDid);
                    qWarning() << msg;

                    if (errorCb)
                        errorCb(msg);
                }
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        session.mAccessJwt);
}

void Client::refreshSession(const ComATProtoServer::Session& session,
                            const SuccessCb& successCb, const ErrorCb& errorCb)
{
    mXrpc->post("com.atproto.server.refreshSession", {},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "Refresh session reply:" << reply;
            try {
                auto refreshed = ComATProtoServer::Session::fromJson(reply);
                if (refreshed->mDid == mSession->mDid)
                {
                    qDebug() << "Session refreshed";
                    mSession = std::move(refreshed);

                    if (successCb)
                        successCb();
                }
                else
                {
                    const auto msg = QString("Session did(%1) does not match refreshed did(%2)").arg(
                        mSession->mDid, refreshed->mDid);
                    qWarning() << msg;

                    if (errorCb)
                        errorCb(msg);
                }
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        refreshToken());
}

void Client::resolveHandle(const QString& handle,
                   const ResolveHandleSuccessCb& successCb, const ErrorCb& errorCb)
{
    mXrpc->get("com.atproto.identity.resolveHandle", {{"handle", handle}},
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
    mXrpc->get("app.bsky.actor.getProfile", {{"actor", user}},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getProfile:" << reply;
            try {
                auto profile = AppBskyActor::ProfileViewDetailed::fromJson(reply);
                if (successCb)
                    successCb(std::move(profile));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getAuthorFeed(const QString& user, std::optional<int> limit, const std::optional<QString>& cursor,
                           const GetAuthorFeedSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"actor", user}};
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    mXrpc->get("app.bsky.feed.getAuthorFeed", params,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getAuthorFeed:" << reply;
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

    mXrpc->get("app.bsky.feed.getTimeline", params,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getTimeline:" << reply;
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

void Client::getPostThread(const QString& uri, std::optional<int> depth, std::optional<int> parentHeight,
                           const GetPostThreadSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"uri", uri}};
    addOptionalIntParam(params, "depth", depth, 0, 1000);
    addOptionalIntParam(params, "parentHeight", parentHeight, 0, 1000);

    mXrpc->get("app.bsky.feed.getPostThread", params,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getPostThread:" << reply;
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
    Q_ASSERT(uris.size() <= 25);
    Xrpc::Client::Params params;

    for (const auto& uri : uris)
        params.append({"uris", uri});

    mXrpc->get("app.bsky.feed.getPosts", params,
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

void Client::getFollows(const QString& actor, std::optional<int> limit, const std::optional<QString>& cursor,
                        const GetFollowsSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"actor", actor}};
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);

    mXrpc->get("app.bsky.graph.getFollows", params,
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

    mXrpc->get("app.bsky.graph.getFollowers", params,
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

void Client::getUnreadNotificationCount(const std::optional<QDateTime>& seenAt,
                                        const UnreadCountSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params;
    addOptionalDateTimeParam(params, "seenAt", seenAt);

    mXrpc->get("app.bsky.notification.getUnreadCount", params,
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

    mXrpc->post("app.bsky.notification.updateSeen", json,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "Updated notification seen:" << reply;
            if (successCb)
                successCb();
        },
        failure(errorCb),
        authToken());
}

void Client::listNotifications(std::optional<int> limit, const std::optional<QString>& cursor,
                       const std::optional<QDateTime>& seenAt,
                       const NotificationsSuccessCb& successCb, const ErrorCb& errorCb,
                       bool updateSeen)
{
    Xrpc::Client::Params params;
    addOptionalIntParam(params, "limit", limit, 1, 100);
    addOptionalStringParam(params, "cursor", cursor);
    addOptionalDateTimeParam(params, "seenAt", seenAt);

    const auto now = QDateTime::currentDateTimeUtc();

    mXrpc->get("app.bsky.notification.listNotifications", params,
        [this, now, successCb, errorCb, updateSeen](const QJsonDocument& reply){
            try {
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

void Client::uploadBlob(const QByteArray& blob, const QString& mimeType,
                        const UploadBlobSuccessCb& successCb, const ErrorCb& errorCb)
{
    mXrpc->post("com.atproto.repo.uploadBlob", blob, mimeType,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "Posted:" << reply;
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

#if 0
    // Test code
    auto blobResult = std::make_unique<ATProto::Blob>();
    blobResult->mRefLink = "TestLink";
    blobResult->mMimeType = mimeType;
    blobResult->mSize = blob.size();
    successCb(std::move(blobResult));
#endif
}

void Client::getRecord(const QString& repo, const QString& collection,
                       const QString& rkey, const std::optional<QString>& cid,
                       const GetRecordSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"repo", repo}, {"collection", collection}, {"rkey", rkey}};
    addOptionalStringParam(params, "cid", cid);

    mXrpc->get("com.atproto.repo.getRecord", params,
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

void Client::createRecord(const QString& repo, const QString& collection, const QJsonObject& record,
                  const CreateRecordSuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject root;
    root.insert("repo", repo);
    root.insert("collection", collection);
    root.insert("record", record);
    QJsonDocument json(root);

    qDebug() << "Create record:" << json;

    mXrpc->post("com.atproto.repo.createRecord", json,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "Created record:" << reply;
            try {
                auto record = ComATProtoRepo::StrongRef::fromJson(reply.object());
                if (successCb)
                    successCb(std::move(record));
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

    mXrpc->post("com.atproto.repo.deleteRecord", jsonDoc,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() <<"Deleted record:" << reply;
            if (successCb)
                successCb();
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
        cb(e.msg());
}

void Client::requestFailed(const QString& err, const QJsonDocument& json, const ErrorCb& errorCb)
{
    qInfo() << "Request failed:" << err;
    qInfo() << json;

    if (json.isNull())
    {
        if (errorCb)
            errorCb(err);

        return;
    }

    try {
        auto error = ATProtoError::fromJson(json);
        if (errorCb)
            errorCb(QString("%1 - %2").arg(error->mError, error->mMessage));
    } catch (InvalidJsonException& e) {
        qWarning() << e.msg();
        if (errorCb)
            errorCb(err);
    }
}

}
