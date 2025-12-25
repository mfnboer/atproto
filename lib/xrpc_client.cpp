// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "xrpc_client.h"

namespace Xrpc {

using namespace std::chrono_literals;

static QNetworkAccessManager* makeNetwork(int networkTransferTimeoutMs, QObject* parent)
{
    auto* network = new QNetworkAccessManager(parent);
    network->setAutoDeleteReplies(true);
    network->setTransferTimeout(networkTransferTimeoutMs);
    return network;
}

Client::Client(const QString& host, int networkTransferTimeoutMs) :
    mNetwork(makeNetwork(networkTransferTimeoutMs, this)),
    mPlcDirectoryClient(mNetwork.get()),
    mIdentityResolver(mNetwork.get()),
    mNetworkThread(new NetworkThread(networkTransferTimeoutMs))
{
    qDebug() << "Host:" << host;
    qDebug() << "Network transfer timeout:" << networkTransferTimeoutMs;
    qDebug() << "Device supports OpenSSL:" << QSslSocket::supportsSsl();
    qDebug() << "OpenSSL lib:" << QSslSocket::sslLibraryVersionString();
    qDebug() << "OpenSSL lib build:" << QSslSocket::sslLibraryBuildVersionString();

    if (!host.isEmpty())
        setPDS(host, "");

    if (mNetworkThread->moveToThread(mNetworkThread.get()))
        qDebug() << "Moved network thread";
    else
        qWarning() << "Failed to move network thread";

    qDebug() << "Thread:" << QThread::currentThreadId();

    connect(mNetworkThread.get(), &NetworkThread::requestSuccessJson, this, &Client::doCallback<NetworkThread::SuccessJsonCb, QJsonDocument>);

    connect(mNetworkThread.get(), &NetworkThread::requestSuccessBytes, this,
        [](QByteArray bytes, NetworkThread::SuccessBytesCb cb, QString contentType) {
            cb(std::move(bytes), std::move(contentType));
        });

    // com.atproto.server
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessSession, this, &Client::doCallback<NetworkThread::SuccessSessionCb, ATProto::ComATProtoServer::Session::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetSessionOutput, this, &Client::doCallback<NetworkThread::SuccessGetSessionOutputCb, ATProto::ComATProtoServer::GetSessionOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetServiceAuthOutput, this, &Client::doCallback<NetworkThread::SuccessGetServiceAuthOutputCb, ATProto::ComATProtoServer::GetServiceAuthOutput::SharedPtr>);

    // com.atproto.identity
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessResolveHandleOutput, this, &Client::doCallback<NetworkThread::SuccessResolveHandleOutputCb, ATProto::ComATProtoIdentity::ResolveHandleOutput::SharedPtr>);

    // app.bsky.actor
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessProfileViewDetailed, this, &Client::doCallback<NetworkThread::SuccessProfileViewDetailedCb, ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetProfilesOutput, this, &Client::doCallback<NetworkThread::SuccessGetProfilesOutputCb, ATProto::AppBskyActor::GetProfilesOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetPreferencesOutput, this, &Client::doCallback<NetworkThread::SuccessGetPreferencesOutputCb, ATProto::AppBskyActor::GetPreferencesOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessSearchActorsOutput, this, &Client::doCallback<NetworkThread::SuccessSearchActorsOutputCb, ATProto::AppBskyActor::SearchActorsOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessSearchActorsTypeaheadOutput, this, &Client::doCallback<NetworkThread::SuccessSearchActorsTypeaheadOutputCb, ATProto::AppBskyActor::SearchActorsTypeaheadOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetSuggestionsOutput, this, &Client::doCallback<NetworkThread::SuccessGetSuggestionsOutputCb, ATProto::AppBskyActor::GetSuggestionsOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetSuggestedFollowsByActor, this, &Client::doCallback<NetworkThread::SuccessGetSuggestedFollowsByActorCb, ATProto::AppBskyActor::GetSuggestedFollowsByActor::SharedPtr>);

    // app.bsky.bookmark
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetBookmarksOutput, this, &Client::doCallback<NetworkThread::SuccessGetBookmarksOutputCb, ATProto::AppBskyBookmark::GetBookmarksOutput::SharedPtr>);

    // app.bsky.labeler
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetServicesOutput, this, &Client::doCallback<NetworkThread::SuccessGetServicesOutputCb, ATProto::AppBskyLabeler::GetServicesOutput::SharedPtr>);

    // app.bsky.feed
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessOutputFeed, this, &Client::doCallback<NetworkThread::SuccessOutputFeedCb, ATProto::AppBskyFeed::OutputFeed::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetFeedGeneratorOutput, this, &Client::doCallback<NetworkThread::SuccessGetFeedGeneratorOutputCb, ATProto::AppBskyFeed::GetFeedGeneratorOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetFeedGeneratorsOutput, this, &Client::doCallback<NetworkThread::SuccessGetFeedGeneratorsOutputCb, ATProto::AppBskyFeed::GetFeedGeneratorsOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetActorFeedsOutput, this, &Client::doCallback<NetworkThread::SuccessGetActorFeedsOutputCb, ATProto::AppBskyFeed::GetActorFeedsOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessPostThread, this, &Client::doCallback<NetworkThread::SuccessPostThreadCb, ATProto::AppBskyFeed::PostThread::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetPostsOutput, this, &Client::doCallback<NetworkThread::SuccessGetPostsOutputCb, ATProto::AppBskyFeed::GetPostsOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetQuotesOutput, this, &Client::doCallback<NetworkThread::SuccessGetQuotesOutputCb, ATProto::AppBskyFeed::GetQuotesOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessSearchPostsOutput, this, &Client::doCallback<NetworkThread::SuccessSearchPostsOutputCb, ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetLikesOutput, this, &Client::doCallback<NetworkThread::SuccessGetLikesOutputCb, ATProto::AppBskyFeed::GetLikesOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetRepostedByOutput, this, &Client::doCallback<NetworkThread::SuccessGetRepostedByOutputCb, ATProto::AppBskyFeed::GetRepostedByOutput::SharedPtr>);

    // app.bsky.graph
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetFollowsOutput, this, &Client::doCallback<NetworkThread::SuccessGetFollowsOutputCb, ATProto::AppBskyGraph::GetFollowsOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetFollowersOutput, this, &Client::doCallback<NetworkThread::SuccessGetFollowersOutputCb, ATProto::AppBskyGraph::GetFollowersOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetBlocksOutput, this, &Client::doCallback<NetworkThread::SuccessGetBlocksOutputCb, ATProto::AppBskyGraph::GetBlocksOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetMutesOutput, this, &Client::doCallback<NetworkThread::SuccessGetMutesOutputCb, ATProto::AppBskyGraph::GetMutesOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetListOutput, this, &Client::doCallback<NetworkThread::SuccessGetListOutputCb, ATProto::AppBskyGraph::GetListOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetListsOutput, this, &Client::doCallback<NetworkThread::SuccessGetListsOutputCb, ATProto::AppBskyGraph::GetListsOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetListsWithMembershipOutput, this, &Client::doCallback<NetworkThread::SuccessGetListsWithMembershipOutputCb, ATProto::AppBskyGraph::GetListsWithMembershipOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetStarterPackOutput, this, &Client::doCallback<NetworkThread::SuccessGetStarterPackOutputCb, ATProto::AppBskyGraph::GetStarterPackOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetStarterPacksOutput, this, &Client::doCallback<NetworkThread::SuccessGetStarterPacksOutputCb, ATProto::AppBskyGraph::GetStarterPacksOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetStarterPacksWithMembershipOutput, this, &Client::doCallback<NetworkThread::SuccessGetStarterPacksWithMembershipOutputCb, ATProto::AppBskyGraph::GetStarterPacksWithMembershipOutput::SharedPtr>);

    // app.bsky.notification
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessListNotificationsOutput, this, &Client::doCallback<NetworkThread::SuccessListNotificationsOutputCb, ATProto::AppBskyNotification::ListNotificationsOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetNotificationPreferencesOutput, this, &Client::doCallback<NetworkThread::SuccessGetNotificationPreferencesOutputCb, ATProto::AppBskyNotification::GetPreferencesOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessListActivitySubscriptionsOutput, this, &Client::doCallback<NetworkThread::SuccessListActivitySubscriptionsOutputCb, ATProto::AppBskyNotification::ListActivitySubscriptionsOutput::SharedPtr>);

    // app.bsky.unspecced
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetSuggestedStarterPacks, this, &Client::doCallback<NetworkThread::SuccessGetSuggestedStarterPacksCb, ATProto::AppBskyUnspecced::GetSuggestedStarterPacksOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetTrends, this, &Client::doCallback<NetworkThread::SuccessGetTrendsCb, ATProto::AppBskyUnspecced::GetTrendsOutput::SharedPtr>);

    // app.bsky.video
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessJobStatusOutput, this, &Client::doCallback<NetworkThread::SuccessJobStatusOutputCb, ATProto::AppBskyVideo::JobStatusOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetUploadLimitsOutput, this, &Client::doCallback<NetworkThread::SuccessGetUploadLimitsOutputCb, ATProto::AppBskyVideo::GetUploadLimitsOutput::SharedPtr>);

    // chat.bsky.convo
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessGetMessagesOutput, this, &Client::doCallback<NetworkThread::SuccessGetMessagesOutputCb, ATProto::ChatBskyConvo::GetMessagesOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessConvoListOutput, this, &Client::doCallback<NetworkThread::SuccessConvoListOutputCb, ATProto::ChatBskyConvo::ConvoListOutput::SharedPtr>);
    connect(mNetworkThread.get(), &NetworkThread::requestSuccessConvoOutput, this, &Client::doCallback<NetworkThread::SuccessConvoOutputCb, ATProto::ChatBskyConvo::ConvoOutput::SharedPtr>);

    connect(mNetworkThread.get(), &NetworkThread::requestError, this,
        [](QString error, QJsonDocument json, NetworkThread::ErrorCb cb) {
            cb(std::move(error), std::move(json));
        });

    connect(mNetworkThread.get(), &NetworkThread::requestInvalidJsonError, this,
        [](QString error, NetworkThread::ErrorCb cb) {
            auto json = QJsonDocument::fromJson("{}");
            cb(std::move(error), std::move(json));
        });

    connect(this, &Client::postDataToNetwork, mNetworkThread.get(), &NetworkThread::postData, Qt::QueuedConnection);
    connect(this, &Client::postJsonToNetwork, mNetworkThread.get(), &NetworkThread::postJson, Qt::QueuedConnection);
    connect(this, &Client::getToNetwork, mNetworkThread.get(), &NetworkThread::get, Qt::QueuedConnection);
    connect(this, &Client::pdsChanged, mNetworkThread.get(), &NetworkThread::setPDS, Qt::QueuedConnection);
    connect(this, &Client::userAgentChanged, mNetworkThread.get(), &NetworkThread::setUserAgent, Qt::QueuedConnection);
    connect(this, &Client::videoHostChanged, mNetworkThread.get(), &NetworkThread::setVideoHost, Qt::QueuedConnection);

    qDebug() << "Start network thread";
    mNetworkThread->start();
}

Client::~Client()
{
    qDebug() << "Destroy client";
    mNetworkThread->exit();
    mNetworkThread->wait(1000);
    qDebug() << "XRPC network thread stopped";
}

template<typename CallbackType, typename ArgType>
void Client::doCallback(ArgType arg, CallbackType cb)
{
    cb(std::move(arg));
}

void Client::setUserAgent(const QString& userAgent)
{
    emit userAgentChanged(userAgent);
}

void Client::setVideoHost(const QString& host)
{
    emit videoHostChanged(host);
}

void Client::setPDS(const QString& pds, const QString& did)
{
    if (pds.startsWith("http"))
        mPDS = pds;
    else
        mPDS = "https://" + pds;

    mDid = did;
    qDebug() << "PDS:" << mPDS << "DID:" << did;
    emit pdsChanged(mPDS);
}

void Client::setPDSFromSession(const ATProto::ComATProtoServer::Session& session)
{
    const auto pds = session.getPDS();

    if (pds)
        setPDS(*pds, session.mDid);
    else
        qDebug() << "No PDS in session, handle:" << session.mHandle << "did:" << session.mDid;
}

void Client::setPDSFromDid(const QString& did, const SetPdsSuccessCb& successCb, const SetPdsErrorCb& errorCb)
{
    qDebug() << "Set PDS from DID:" << did;

    if (!mPDS.isEmpty() && mDid == did)
    {
        qDebug() << "PDS already set:" << mPDS << "DID:" << did;
        QTimer::singleShot(0, this, [successCb]{
            if (successCb)
                successCb();
        });

        return;
    }

    mPlcDirectoryClient.getPds(did,
        [this, presence=getPresence(), did, successCb](const QString& pds){
            if (!presence)
                return;

            setPDS(pds, did);

            if (successCb)
                successCb();
        },
        [this, did, presence=getPresence(), successCb, errorCb](int errorCode, const QString& error){
            if (!presence)
                return;

            qWarning() << "Failed to set PDS:" << did << errorCode << error;

            if (!mPDS.isEmpty())
            {
                qDebug() << "Initial point of contact:" << mPDS;

                if (successCb)
                    successCb();
            }
            else
            {
                if (errorCb)
                    errorCb(QString("Could not get PDS: %1 %2, DID: %3").arg(errorCode).arg(error, did));
            }
        });
}

void Client::setPDSFromHandle(const QString& handle, const SetPdsSuccessCb& successCb, const SetPdsErrorCb& errorCb)
{
    qDebug() << "Set PDS from handle:" << handle;

    mIdentityResolver.resolveHandle(handle,
        [this, presence=getPresence(), successCb, errorCb](const QString& did){
            if (!presence)
                return;

            setPDSFromDid(did, successCb, errorCb);
        },
        [this, presence=getPresence(), handle, successCb, errorCb](const QString& error){
            if (!presence)
                return;

            qWarning() << "Failed resolve handle:" << handle << "error:" << error;

            if (!mPDS.isEmpty())
            {
                qDebug() << "Initial point of contact:" << mPDS;

                if (successCb)
                    successCb();
            }
            else
            {
                if (errorCb)
                    errorCb(error);
            }
        });
}

void Client::post(const QString& service, const QJsonDocument& json, const NetworkThread::Params& rawHeaders,
                  const NetworkThread::CallbackType& successCb, const NetworkThread::ErrorCb& errorCb, const QString& accessJwt)
{
    Q_ASSERT(!service.isEmpty());
    Q_ASSERT(errorCb);
    emit postJsonToNetwork(service, json, rawHeaders, successCb, errorCb, accessJwt);
}

void Client::post(const QString& service, const NetworkThread::DataType& data, const QString& mimeType, const NetworkThread::Params& rawHeaders,
                  const NetworkThread::SuccessJsonCb& successCb, const NetworkThread::ErrorCb& errorCb, const QString& accessJwt)
{
    Q_ASSERT(!service.isEmpty());
    Q_ASSERT(successCb);
    Q_ASSERT(errorCb);
    emit postDataToNetwork(service, data, mimeType, rawHeaders, successCb, errorCb, accessJwt);
}

void Client::get(const QString& service, const NetworkThread::Params& params, const NetworkThread::Params& rawHeaders,
                 const NetworkThread::CallbackType& successCb, const NetworkThread::ErrorCb& errorCb, const QString& accessJwt, const QString& pds)
{
    Q_ASSERT(!service.isEmpty());
    Q_ASSERT(errorCb);
    emit getToNetwork(service, params, rawHeaders, successCb, errorCb, accessJwt, pds);
}

}
