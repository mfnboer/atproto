// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "lexicon/app_bsky_actor.h"
#include "lexicon/app_bsky_feed.h"
#include "lexicon/app_bsky_graph.h"
#include "lexicon/app_bsky_labeler.h"
#include "lexicon/app_bsky_notification.h"
#include "lexicon/app_bsky_video.h"
#include "lexicon/chat_bsky_convo.h"
#include "lexicon/com_atproto_identity.h"
#include "lexicon/com_atproto_server.h"
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QThread>

namespace Xrpc {

class NetworkThread : public QThread
{
    Q_OBJECT
public:
    using DataType = std::variant<QByteArray, QIODevice*>;
    using Params = QList<QPair<QString, QString>>;
    using ErrorCb = std::function<void(const QString& err, const QJsonDocument& json)>;
    using SuccessJsonCb = std::function<void(const QJsonDocument& json)>;
    using SuccessBytesCb = std::function<void(const QByteArray& bytes, const QString& contentType)>;

    // com.atproto.server
    using SuccessSessionCb = std::function<void(ATProto::ComATProtoServer::Session::SharedPtr)>;
    using SuccessGetSessionOutputCb = std::function<void(ATProto::ComATProtoServer::GetSessionOutput::SharedPtr)>;
    using SuccessGetServiceAuthOutputCb = std::function<void(ATProto::ComATProtoServer::GetServiceAuthOutput::SharedPtr)>;

    // com.atproto.identity
    using SuccessResolveHandleOutputCb = std::function<void(ATProto::ComATProtoIdentity::ResolveHandleOutput::SharedPtr)>;

    // app.bsky.actor
    using SuccessProfileViewDetailedCb = std::function<void(ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr)>;
    using SuccessGetPreferencesOutputCb = std::function<void(ATProto::AppBskyActor::GetPreferencesOutput::SharedPtr)>;
    using SuccessSearchActorsOutputCb = std::function<void(ATProto::AppBskyActor::SearchActorsOutput::SharedPtr)>;
    using SuccessSearchActorsTypeaheadOutputCb = std::function<void(ATProto::AppBskyActor::SearchActorsTypeaheadOutput::SharedPtr)>;
    using SuccessGetSuggestionsOutputCb = std::function<void(ATProto::AppBskyActor::GetSuggestionsOutput::SharedPtr)>;
    using SuccessGetSuggestedFollowsByActorCb = std::function<void(ATProto::AppBskyActor::GetSuggestedFollowsByActor::SharedPtr)>;

    // app.bsky.labeler
    using SuccessGetServicesOutputCb = std::function<void(ATProto::AppBskyLabeler::GetServicesOutput::SharedPtr)>;

    // app.bsky.feed
    using SuccessOutputFeedCb = std::function<void(ATProto::AppBskyFeed::OutputFeed::SharedPtr)>;
    using SuccessGetFeedGeneratorOutputCb = std::function<void(ATProto::AppBskyFeed::GetFeedGeneratorOutput::SharedPtr)>;
    using SuccessGetFeedGeneratorsOutputCb = std::function<void(ATProto::AppBskyFeed::GetFeedGeneratorsOutput::SharedPtr)>;
    using SuccessGetActorFeedsOutputCb = std::function<void(ATProto::AppBskyFeed::GetActorFeedsOutput::SharedPtr)>;
    using SuccessPostThreadCb = std::function<void(ATProto::AppBskyFeed::PostThread::SharedPtr)>;
    using SuccessGetQuotesOutputCb = std::function<void(ATProto::AppBskyFeed::GetQuotesOutput::SharedPtr)>;
    using SuccessSearchPostsOutputCb = std::function<void(ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr)>;
    using SuccessGetLikesOutputCb = std::function<void(ATProto::AppBskyFeed::GetLikesOutput::SharedPtr)>;
    using SuccessGetRepostedByOutputCb = std::function<void(ATProto::AppBskyFeed::GetRepostedByOutput::SharedPtr)>;

    // app.bsky.graph
    using SuccessGetFollowsOutputCb = std::function<void(ATProto::AppBskyGraph::GetFollowsOutput::SharedPtr)>;
    using SuccessGetFollowersOutputCb = std::function<void(ATProto::AppBskyGraph::GetFollowersOutput::SharedPtr)>;
    using SuccessGetBlocksOutputCb = std::function<void(ATProto::AppBskyGraph::GetBlocksOutput::SharedPtr)>;
    using SuccessGetMutesOutputCb = std::function<void(ATProto::AppBskyGraph::GetMutesOutput::SharedPtr)>;
    using SuccessGetListOutputCb = std::function<void(ATProto::AppBskyGraph::GetListOutput::SharedPtr)>;
    using SuccessGetListsOutputCb = std::function<void(ATProto::AppBskyGraph::GetListsOutput::SharedPtr)>;
    using SuccessGetStarterPackOutputCb = std::function<void(ATProto::AppBskyGraph::GetStarterPackOutput::SharedPtr)>;
    using SuccessGetStarterPacksOutputCb = std::function<void(ATProto::AppBskyGraph::GetStarterPacksOutput::SharedPtr)>;

    // app.bsky.notification
    using SuccessListNotificationsOutputCb = std::function<void(ATProto::AppBskyNotification::ListNotificationsOutput::SharedPtr)>;

    // app.bsky.video
    using SuccessJobStatusOutputCb = std::function<void(ATProto::AppBskyVideo::JobStatusOutput::SharedPtr)>;
    using SuccessGetUploadLimitsOutputCb = std::function<void(ATProto::AppBskyVideo::GetUploadLimitsOutput::SharedPtr)>;

    // chat.bsky.convo
    using SuccessGetMessagesOutputCb = std::function<void(ATProto::ChatBskyConvo::GetMessagesOutput::SharedPtr)>;
    using SuccessConvoListOutputCb = std::function<void(ATProto::ChatBskyConvo::ConvoListOutput::SharedPtr)>;
    using SuccessConvoOutputCb = std::function<void(ATProto::ChatBskyConvo::ConvoOutput::SharedPtr)>;

    using CallbackType = std::variant<
        SuccessJsonCb,
        SuccessBytesCb,

        // com.atproto.server
        SuccessSessionCb,
        SuccessGetSessionOutputCb,
        SuccessGetServiceAuthOutputCb,

        // com.atproto.identity
        SuccessResolveHandleOutputCb,

        // app.bsky.actor
        SuccessProfileViewDetailedCb,
        SuccessGetPreferencesOutputCb,
        SuccessSearchActorsOutputCb,
        SuccessSearchActorsTypeaheadOutputCb,
        SuccessGetSuggestionsOutputCb,
        SuccessGetSuggestedFollowsByActorCb,

        // app.bsky.labeler
        SuccessGetServicesOutputCb,

        // app.bsky.feed
        SuccessOutputFeedCb,
        SuccessGetFeedGeneratorOutputCb,
        SuccessGetFeedGeneratorsOutputCb,
        SuccessGetActorFeedsOutputCb,
        SuccessPostThreadCb,
        SuccessGetQuotesOutputCb,
        SuccessSearchPostsOutputCb,
        SuccessGetLikesOutputCb,
        SuccessGetRepostedByOutputCb,

        // app.bsky.graph
        SuccessGetFollowsOutputCb,
        SuccessGetFollowersOutputCb,
        SuccessGetBlocksOutputCb,
        SuccessGetMutesOutputCb,
        SuccessGetListOutputCb,
        SuccessGetListsOutputCb,
        SuccessGetStarterPackOutputCb,
        SuccessGetStarterPacksOutputCb,

        // app.bsky.notification
        SuccessListNotificationsOutputCb,

        // app.bsky.video
        SuccessJobStatusOutputCb,
        SuccessGetUploadLimitsOutputCb,

        // chat.bsky.convo
        SuccessGetMessagesOutputCb,
        SuccessConvoListOutputCb,
        SuccessConvoOutputCb
    >;

    struct Request
    {
        bool mIsPost = false;
        QNetworkRequest mXrpcRequest;
        DataType mData;
        int mResendCount = 0;
    };

    NetworkThread(QObject* parent = nullptr);
    void setPDS(const QString& pds) { mPDS = pds; }
    void setUserAgent(const QString& userAgent) { mUserAgent = userAgent; }
    void postData(const QString& service, const DataType& data, const QString& mimeType, const Params& rawHeaders,
                  const CallbackType& successCb, const ErrorCb& errorCb, const QString& accessJwt);
    void postJson(const QString& service, const QJsonDocument& json, const Params& rawHeaders,
                  const CallbackType& successCb, const ErrorCb& errorCb, const QString& accessJwt);
    void get(const QString& service, const Params& params, const Params& rawHeaders,
             const CallbackType& successCb, const ErrorCb& errorCb, const QString& accessJwt);

signals:
    // clazy:excludeall=fully-qualified-moc-types
    void requestSuccessJson(QJsonDocument json, SuccessJsonCb cb);
    void requestSuccessBytes(QByteArray bytes, SuccessBytesCb cb, QString contentType);

    // com.atproto.server
    void requestSuccessSession(ATProto::ComATProtoServer::Session::SharedPtr, SuccessSessionCb);
    void requestSuccessGetSessionOutput(ATProto::ComATProtoServer::GetSessionOutput::SharedPtr, SuccessGetSessionOutputCb);
    void requestSuccessGetServiceAuthOutput(ATProto::ComATProtoServer::GetServiceAuthOutput::SharedPtr, SuccessGetServiceAuthOutputCb);

    // com.atproto.identity
    void requestSuccessResolveHandleOutput(ATProto::ComATProtoIdentity::ResolveHandleOutput::SharedPtr, SuccessResolveHandleOutputCb);

    // app.bsky.actor
    void requestSuccessProfileViewDetailed(ATProto::AppBskyActor::ProfileViewDetailed::SharedPtr, SuccessProfileViewDetailedCb);
    void requestSuccessGetPreferencesOutput(ATProto::AppBskyActor::GetPreferencesOutput::SharedPtr, SuccessGetPreferencesOutputCb);
    void requestSuccessSearchActorsOutput(ATProto::AppBskyActor::SearchActorsOutput::SharedPtr, SuccessSearchActorsOutputCb);
    void requestSuccessSearchActorsTypeaheadOutput(ATProto::AppBskyActor::SearchActorsTypeaheadOutput::SharedPtr, SuccessSearchActorsTypeaheadOutputCb);
    void requestSuccessGetSuggestionsOutput(ATProto::AppBskyActor::GetSuggestionsOutput::SharedPtr, SuccessGetSuggestionsOutputCb);
    void requestSuccessGetSuggestedFollowsByActor(ATProto::AppBskyActor::GetSuggestedFollowsByActor::SharedPtr, SuccessGetSuggestedFollowsByActorCb);

    // app.bsky.labeler
    void requestSuccessGetServicesOutput(ATProto::AppBskyLabeler::GetServicesOutput::SharedPtr, SuccessGetServicesOutputCb);

    // app.bsky.feed
    void requestSuccessOutputFeed(ATProto::AppBskyFeed::OutputFeed::SharedPtr, SuccessOutputFeedCb);
    void requestSuccessGetFeedGeneratorOutput(ATProto::AppBskyFeed::GetFeedGeneratorOutput::SharedPtr, SuccessGetFeedGeneratorOutputCb);
    void requestSuccessGetFeedGeneratorsOutput(ATProto::AppBskyFeed::GetFeedGeneratorsOutput::SharedPtr, SuccessGetFeedGeneratorsOutputCb);
    void requestSuccessGetActorFeedsOutput(ATProto::AppBskyFeed::GetActorFeedsOutput::SharedPtr, SuccessGetActorFeedsOutputCb);
    void requestSuccessPostThread(ATProto::AppBskyFeed::PostThread::SharedPtr, SuccessPostThreadCb);
    void requestSuccessGetQuotesOutput(ATProto::AppBskyFeed::GetQuotesOutput::SharedPtr, SuccessGetQuotesOutputCb);
    void requestSuccessSearchPostsOutput(ATProto::AppBskyFeed::SearchPostsOutput::SharedPtr, SuccessSearchPostsOutputCb);
    void requestSuccessGetLikesOutput(ATProto::AppBskyFeed::GetLikesOutput::SharedPtr, SuccessGetLikesOutputCb);
    void requestSuccessGetRepostedByOutput(ATProto::AppBskyFeed::GetRepostedByOutput::SharedPtr, SuccessGetRepostedByOutputCb);

    // app.bsky.graph
    void requestSuccessGetFollowsOutput(ATProto::AppBskyGraph::GetFollowsOutput::SharedPtr, SuccessGetFollowsOutputCb);
    void requestSuccessGetFollowersOutput(ATProto::AppBskyGraph::GetFollowersOutput::SharedPtr, SuccessGetFollowersOutputCb);
    void requestSuccessGetBlocksOutput(ATProto::AppBskyGraph::GetBlocksOutput::SharedPtr, SuccessGetBlocksOutputCb);
    void requestSuccessGetMutesOutput(ATProto::AppBskyGraph::GetMutesOutput::SharedPtr, SuccessGetMutesOutputCb);
    void requestSuccessGetListOutput(ATProto::AppBskyGraph::GetListOutput::SharedPtr, SuccessGetListOutputCb);
    void requestSuccessGetListsOutput(ATProto::AppBskyGraph::GetListsOutput::SharedPtr, SuccessGetListsOutputCb);
    void requestSuccessGetStarterPackOutput(ATProto::AppBskyGraph::GetStarterPackOutput::SharedPtr, SuccessGetStarterPackOutputCb);
    void requestSuccessGetStarterPacksOutput(ATProto::AppBskyGraph::GetStarterPacksOutput::SharedPtr, SuccessGetStarterPacksOutputCb);

    // app.bsky.notification
    void requestSuccessListNotificationsOutput(ATProto::AppBskyNotification::ListNotificationsOutput::SharedPtr, SuccessListNotificationsOutputCb);

    // app.bsky.video
    void requestSuccessJobStatusOutput(ATProto::AppBskyVideo::JobStatusOutput::SharedPtr, SuccessJobStatusOutputCb);
    void requestSuccessGetUploadLimitsOutput(ATProto::AppBskyVideo::GetUploadLimitsOutput::SharedPtr, SuccessGetUploadLimitsOutputCb);

    // chat.bsky.convo
    void requestSuccessGetMessagesOutput(ATProto::ChatBskyConvo::GetMessagesOutput::SharedPtr, SuccessGetMessagesOutputCb);
    void requestSuccessConvoListOutput(ATProto::ChatBskyConvo::ConvoListOutput::SharedPtr, SuccessConvoListOutputCb);
    void requestSuccessConvoOutput(ATProto::ChatBskyConvo::ConvoOutput::SharedPtr, SuccessConvoOutputCb);

    void requestError(QString error, QJsonDocument json, ErrorCb cb);
    void requestInvalidJsonError(QString exceptionMsg, ErrorCb cb);

protected:
    virtual void run() override;

private:
    QUrl buildUrl(const QString& service) const;
    QUrl buildUrl(const QString& service, const Params& params) const;
    void setUserAgentHeader(QNetworkRequest& request) const;
    void setAuthorization(QNetworkRequest& request, const QString& accessJwt) const;
    void setRawHeaders(QNetworkRequest& request, const Params& params) const;

    void sendRequest(const Request& request, const CallbackType& successCb, const ErrorCb& errorCb);
    bool resendRequest(Request request, const CallbackType& successCb, const ErrorCb& errorCb);
    bool mustResend(QNetworkReply::NetworkError error) const;
    void invokeCallback(CallbackType successCb, const ErrorCb& errorCb, QByteArray data, const QString& contentType);
    void replyFinished(const Request& request, QNetworkReply* reply,
                       CallbackType successCb, const ErrorCb& errorCb,
                       std::shared_ptr<bool> errorHandled);
    void networkError(const Request& request, QNetworkReply* reply, QNetworkReply::NetworkError errorCode,
                      const CallbackType& successCb, const ErrorCb& errorCb,
                      std::shared_ptr<bool> errorHandled);
    void sslErrors(QNetworkReply* reply, const QList<QSslError>& errors, const ErrorCb& errorCb, std::shared_ptr<bool> errorHandled);

    struct Task
    {
        Request mRequest;
        CallbackType mCb;
    };

    QNetworkAccessManager* mNetwork;
    QString mPDS;
    QString mUserAgent;
};

}
