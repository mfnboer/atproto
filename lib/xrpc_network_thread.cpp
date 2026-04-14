// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "xrpc_network_thread.h"
#include "client.h"
#include "network_utils.h"
#include "xjson.h"
#include "lexicon/lexicon.h"
#include <QSslSocket>

namespace Xrpc {

using namespace std::chrono_literals;

constexpr int MAX_RESEND = 4;
static constexpr int MAX_DPOP_RESEND = 2;

static bool isEmpty(const NetworkThread::DataType& data)
{
    if (std::holds_alternative<QByteArray>(data))
    {
        const auto& bytes = std::get<QByteArray>(data);
        return bytes.isEmpty();
    }
    else
    {
        auto* ioDevice = std::get<QIODevice*>(data);
        return ioDevice->atEnd();
    }
}


NetworkThread::NetworkThread(int networkTransferTimeoutMs, QObject* parent) :
    QThread(parent),
    mNetworkTransferTimeoutMs(networkTransferTimeoutMs),
    mVideoHost(ATProto::Client::SERVICE_VIDEO_HOST)
{
}

void NetworkThread::setPDS(const QString& pds)
{
    mPDS = pds;

    if (mOAuth)
        mOAuth->setPds(pds);
}

void NetworkThread::setUserAgent(const QString& userAgent)
{
    mUserAgent = userAgent;

    if (mOAuth)
        mOAuth->setUserAgent(userAgent);
}

void NetworkThread::setVideoHost(const QString& host)
{
    qDebug() << "Video host:" << host;
    mVideoHost = host;
}

void NetworkThread::run()
{
    qDebug() << "XRPC network thread running:" << currentThreadId();
    mNetwork = new QNetworkAccessManager(this);
    mNetwork->setAutoDeleteReplies(true);
    mNetwork->setTransferTimeout(mNetworkTransferTimeoutMs);
    exec();
}

void NetworkThread::postData(const QString& service, const DataType& data, const QString& mimeType, const Params& rawHeaders,
              const CallbackType& successCb, const ErrorCb& errorCb, const QString& accessJwt)
{
    Request request;
    request.mIsPost = true;
    request.mXrpcRequest = QNetworkRequest(buildUrl(service));
    setUserAgentHeader(request.mXrpcRequest);

    if (!accessJwt.isNull())
        setAuthorization(request, accessJwt);

    // Setting Content-Type header when no body is present causes this error on some
    // PDS' as of 23-6-2024
    if (!isEmpty(data))
        request.mXrpcRequest.setHeader(QNetworkRequest::ContentTypeHeader, mimeType);

    setRawHeaders(request.mXrpcRequest, rawHeaders);
    request.mData = data;
    sendRequest(request, successCb, errorCb);
}

void NetworkThread::postJson(const QString& service, const QJsonDocument& json, const Params& rawHeaders,
              const CallbackType& successCb, const ErrorCb& errorCb, const QString& accessJwt)
{
    const QByteArray data(json.toJson(QJsonDocument::Compact));
    postData(service, data, "application/json", rawHeaders, successCb, errorCb, accessJwt);
}

void NetworkThread::get(const QString& service, const Params& params, const Params& rawHeaders,
         const CallbackType& successCb, const ErrorCb& errorCb, const QString& accessJwt,
         const QString& pds)
{
    Request request;
    request.mIsPost = false;
    request.mXrpcRequest = QNetworkRequest(buildUrl(service, params, pds));
    setUserAgentHeader(request.mXrpcRequest);

    if (!accessJwt.isNull())
        setAuthorization(request, accessJwt);

    setRawHeaders(request.mXrpcRequest, rawHeaders);
    sendRequest(request, successCb, errorCb);
}

void NetworkThread::sendRequest(Request& request, const CallbackType& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Request:" << request.mXrpcRequest.url()  << "Thread:" << currentThreadId();
    QNetworkReply* reply;
    const QString host = request.mXrpcRequest.url().host();

    // HACK:
    // HTTP/2 does not work between Qt6.10.2 and Eurosky. Connection goes
    // stale after a while and then all requests time out for ~25s.
    // With HTTP/1.1 all works fine.
    // With Qt6.8.3 HTTP/2 worked fine too.

    if (host.endsWith("eurosky.social"))
        request.mXrpcRequest.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);

    if (request.mIsPost)
    {
        if (std::holds_alternative<QByteArray>(request.mData))
        {
            const auto& bytes = std::get<QByteArray>(request.mData);
            reply = mNetwork->post(request.mXrpcRequest, bytes);
        }
        else
        {
            auto* ioDevice = std::get<QIODevice*>(request.mData);
            reply = mNetwork->post(request.mXrpcRequest, ioDevice);
        }
    }
    else
    {
        reply = mNetwork->get(request.mXrpcRequest);
    }

    request.mSendTime = QDateTime::currentDateTime();

    // In case of an error multiple callbacks may fire. First errorOcccured() and then probably finished()
    // The latter call is not guaranteed however. We must only call errorCb once!
    auto errorHandled = std::make_shared<bool>(false);

    connect(reply, &QNetworkReply::finished, this,
            [this, request, reply, successCb, errorCb, errorHandled]{ replyFinished(request, reply, std::move(successCb), errorCb, errorHandled); });
    connect(reply, &QNetworkReply::errorOccurred, this,
            [this, request, reply, successCb, errorCb, errorHandled](auto errorCode){ this->networkError(request, reply, errorCode, successCb, errorCb, errorHandled); });
    connect(reply, &QNetworkReply::sslErrors, this,
            [this, reply, errorCb, errorHandled](const QList<QSslError>& errors){ sslErrors(reply, errors, errorCb, errorHandled); });
}

void NetworkThread::replyFinished(const Request& request, QNetworkReply* reply,
                   CallbackType successCb, const ErrorCb& errorCb,
                   std::shared_ptr<bool> errorHandled)
{
    Q_ASSERT(reply);
    const auto errorCode = reply->error();
    const QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString().trimmed();
    const auto respondeDt = QDateTime::currentDateTime() - request.mSendTime;
    qDebug() << "Reply:" << errorCode << "content:" << contentType << "errorHandled:" << *errorHandled << "responseMs:" << (respondeDt / 1ms) << request.mXrpcRequest.url();
    auto data = reply->readAll();
    const bool hasDpopNonce = ATProto::NetworkUtils::hasDpopNonce(reply);

    if (hasDpopNonce)
        mDpopPdsNonce = ATProto::NetworkUtils::getDpopNonce(reply);

    // WORK AROUND:
    // Since Qt6.9.2 we sometimes get an Unknown error like this:
    // 09-01 19:24:47.661 10707 10801 W qt.network.http2: 19:24:47.662 warning unknown'0 stream 7 error: "Received GOAWAY"
    // 09-01 19:24:47.662 10707 10801 W qt.network.http2: 19:24:47.662 warning unknown'0 stream 7 finished with error: ""
    // 09-01 19:24:47.662 10707 10792 I default : 19:24:47.662 info unknown'0 Network error: QNetworkReply::NoError "Unknown error"
    // 09-01 19:24:47.662 10707 10792 W default : 19:24:47.663 warning unknown'0 Retry on unknown error
    if (errorCode == QNetworkReply::NoError && !*errorHandled)
    {
        invokeCallback(std::move(successCb), errorCb, std::move(data), contentType);
    }
    else if (!*errorHandled)
    {
        *errorHandled = true;

        if (mustResend(errorCode))
        {
            if (resendRequest(request, successCb, errorCb))
                return;
        }
        else if (ATProto::NetworkUtils::isDpopNonceError(reply, data))
        {
            if (hasDpopNonce)
            {
                resendWithNewDpopNonce(request, successCb, errorCb);
            }
            else
            {
                qWarning() << "DPoP-Nonce missing";
                emit requestError(ATProto::ATProtoErrorMsg::DPOP_NONCE_MISSING, {}, errorCb);
            }

            return;
        }

        QJsonDocument json(QJsonDocument::fromJson(data));
        emit requestError(reply->errorString(), std::move(json), errorCb);
    }
    else
    {
        qDebug() << "Error already handled";
    }
}

void NetworkThread::networkError(const Request& request, QNetworkReply* reply, QNetworkReply::NetworkError errorCode,
                  const CallbackType& successCb, const ErrorCb& errorCb,
                  std::shared_ptr<bool> errorHandled)
{
    Q_ASSERT(reply);
    auto errorMsg = reply->errorString();
    qInfo() << "Network error:" << errorCode << errorMsg;

    if (!*errorHandled)
    {
        *errorHandled = true;
        const auto data = reply->readAll();

        if (errorCode == QNetworkReply::OperationCanceledError)
            reply->disconnect();

        if (mustResend(errorCode))
        {
            qDebug() << "Try resend on error:" << errorCode << errorMsg;

            if (resendRequest(request, successCb, errorCb))
                return;
        }
        else if (ATProto::NetworkUtils::isDpopNonceError(reply, data))
        {
            if (ATProto::NetworkUtils::hasDpopNonce(reply))
            {
                mDpopPdsNonce = ATProto::NetworkUtils::getDpopNonce(reply);
                resendWithNewDpopNonce(request, successCb, errorCb);
            }
            else
            {
                qWarning() << "DPoP-Nonce missing";
                emit requestError(ATProto::ATProtoErrorMsg::DPOP_NONCE_MISSING, {}, errorCb);
            }

            return;
        }

        if (errorCode == QNetworkReply::OperationCanceledError)
        {
            emit requestError(ATProto::ATProtoErrorMsg::XRPC_TIMEOUT, {}, errorCb);
            return;
        }

        QJsonDocument json(QJsonDocument::fromJson(data));
        emit requestError(std::move(errorMsg), std::move(json), errorCb);
    }
    else
    {
        qDebug() << "Error already handled";
    }
}

void NetworkThread::sslErrors(QNetworkReply* reply, const QList<QSslError>& errors, const ErrorCb& errorCb, std::shared_ptr<bool> errorHandled)
{
    Q_ASSERT(reply);
    qWarning() << "SSL errors:" << errors;

    if (!*errorHandled)
    {
        *errorHandled = true;
        QString msg = "SSL error";

        if (!errors.empty())
            msg.append(": ").append(errors.front().errorString());

        emit requestError(std::move(msg), {}, errorCb);
    }
    else
    {
        qDebug() << "Error already handled";
    }
}

template<typename T, typename Enable = void>
struct FromJson {};

// com.atproto.server

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessSessionCb>>>
{
    using ReplyType = ATProto::ComATProtoServer::Session;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessSession;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetSessionOutputCb>>>
{
    using ReplyType = ATProto::ComATProtoServer::GetSessionOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetSessionOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetServiceAuthOutputCb>>>
{
    using ReplyType = ATProto::ComATProtoServer::GetServiceAuthOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetServiceAuthOutput;
};


// com.atproto.identity

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessResolveHandleOutputCb>>>
{
    using ReplyType = ATProto::ComATProtoIdentity::ResolveHandleOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessResolveHandleOutput;
};


// app.bsky.actor

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessProfileViewDetailedCb>>>
{
    using ReplyType = ATProto::AppBskyActor::ProfileViewDetailed;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessProfileViewDetailed;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetProfilesOutputCb>>>
{
    using ReplyType = ATProto::AppBskyActor::GetProfilesOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetProfilesOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetPreferencesOutputCb>>>
{
    using ReplyType = ATProto::AppBskyActor::GetPreferencesOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetPreferencesOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessSearchActorsOutputCb>>>
{
    using ReplyType = ATProto::AppBskyActor::SearchActorsOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessSearchActorsOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessSearchActorsTypeaheadOutputCb>>>
{
    using ReplyType = ATProto::AppBskyActor::SearchActorsTypeaheadOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessSearchActorsTypeaheadOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetSuggestionsOutputCb>>>
{
    using ReplyType = ATProto::AppBskyActor::GetSuggestionsOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetSuggestionsOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetSuggestedFollowsByActorCb>>>
{
    using ReplyType = ATProto::AppBskyActor::GetSuggestedFollowsByActor;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetSuggestedFollowsByActor;
};


// app.bsky.bookmark

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetBookmarksOutputCb>>>
{
    using ReplyType = ATProto::AppBskyBookmark::GetBookmarksOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetBookmarksOutput;
};


// app.bsky.labeler

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetServicesOutputCb>>>
{
    using ReplyType = ATProto::AppBskyLabeler::GetServicesOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetServicesOutput;
};


// app.bsky.feed

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessOutputFeedCb>>>
{
    using ReplyType = ATProto::AppBskyFeed::OutputFeed;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessOutputFeed;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetFeedGeneratorOutputCb>>>
{
    using ReplyType = ATProto::AppBskyFeed::GetFeedGeneratorOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetFeedGeneratorOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetFeedGeneratorsOutputCb>>>
{
    using ReplyType = ATProto::AppBskyFeed::GetFeedGeneratorsOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetFeedGeneratorsOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetActorFeedsOutputCb>>>
{
    using ReplyType = ATProto::AppBskyFeed::GetActorFeedsOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetActorFeedsOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessPostThreadCb>>>
{
    using ReplyType = ATProto::AppBskyFeed::PostThread;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessPostThread;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetPostsOutputCb>>>
{
    using ReplyType = ATProto::AppBskyFeed::GetPostsOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetPostsOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetQuotesOutputCb>>>
{
    using ReplyType = ATProto::AppBskyFeed::GetQuotesOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetQuotesOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessSearchPostsOutputCb>>>
{
    using ReplyType = ATProto::AppBskyFeed::SearchPostsOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessSearchPostsOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetLikesOutputCb>>>
{
    using ReplyType = ATProto::AppBskyFeed::GetLikesOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetLikesOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetRepostedByOutputCb>>>
{
    using ReplyType = ATProto::AppBskyFeed::GetRepostedByOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetRepostedByOutput;
};


// app.bky.draft

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetDraftsOutputCb>>>
{
    using ReplyType = ATProto::AppBskyDraft::GetDraftsOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetDraftsOutput;
};


// app.bsky.graph

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetFollowsOutputCb>>>
{
    using ReplyType = ATProto::AppBskyGraph::GetFollowsOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetFollowsOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetFollowersOutputCb>>>
{
    using ReplyType = ATProto::AppBskyGraph::GetFollowersOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetFollowersOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetBlocksOutputCb>>>
{
    using ReplyType = ATProto::AppBskyGraph::GetBlocksOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetBlocksOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetMutesOutputCb>>>
{
    using ReplyType = ATProto::AppBskyGraph::GetMutesOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetMutesOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetListOutputCb>>>
{
    using ReplyType = ATProto::AppBskyGraph::GetListOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetListOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetListsOutputCb>>>
{
    using ReplyType = ATProto::AppBskyGraph::GetListsOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetListsOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetListsWithMembershipOutputCb>>>
{
    using ReplyType = ATProto::AppBskyGraph::GetListsWithMembershipOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetListsWithMembershipOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetStarterPackOutputCb>>>
{
    using ReplyType = ATProto::AppBskyGraph::GetStarterPackOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetStarterPackOutput;
};


template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetStarterPacksOutputCb>>>
{
    using ReplyType = ATProto::AppBskyGraph::GetStarterPacksOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetStarterPacksOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetStarterPacksWithMembershipOutputCb>>>
{
    using ReplyType = ATProto::AppBskyGraph::GetStarterPacksWithMembershipOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetStarterPacksWithMembershipOutput;
};


// app.bsky.notification

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessListNotificationsOutputCb>>>
{
    using ReplyType = ATProto::AppBskyNotification::ListNotificationsOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessListNotificationsOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetNotificationPreferencesOutputCb>>>
{
    using ReplyType = ATProto::AppBskyNotification::GetPreferencesOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetNotificationPreferencesOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessListActivitySubscriptionsOutputCb>>>
{
    using ReplyType = ATProto::AppBskyNotification::ListActivitySubscriptionsOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessListActivitySubscriptionsOutput;
};

// app.bsky.unspecced

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetSuggestedStarterPacksCb>>>
{
    using ReplyType = ATProto::AppBskyUnspecced::GetSuggestedStarterPacksOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetSuggestedStarterPacks;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetTrendsCb>>>
{
    using ReplyType = ATProto::AppBskyUnspecced::GetTrendsOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetTrends;
};

// app.bsky.video

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessJobStatusOutputCb>>>
{
    using ReplyType = ATProto::AppBskyVideo::JobStatusOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessJobStatusOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetUploadLimitsOutputCb>>>
{
    using ReplyType = ATProto::AppBskyVideo::GetUploadLimitsOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetUploadLimitsOutput;
};


// chat.bsky.convo

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessGetMessagesOutputCb>>>
{
    using ReplyType = ATProto::ChatBskyConvo::GetMessagesOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessGetMessagesOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessConvoListOutputCb>>>
{
    using ReplyType = ATProto::ChatBskyConvo::ConvoListOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessConvoListOutput;
};

template<typename T>
struct FromJson<T, typename std::enable_if_t<std::is_same_v<T, NetworkThread::SuccessConvoOutputCb>>>
{
    using ReplyType = ATProto::ChatBskyConvo::ConvoOutput;
    static constexpr auto sEmitFun = &NetworkThread::requestSuccessConvoOutput;
};

void NetworkThread::invokeCallback(CallbackType successCb, const ErrorCb& errorCb, QByteArray data, const QString& contentType)
{
    std::visit(
        [this, errorCb, data, contentType](auto&& cb){
            using T = std::decay_t<decltype(cb)>;

            if constexpr (std::is_same_v<T, SuccessBytesCb>)
            {
                emit requestSuccessBytes(std::move(data), std::move(cb), contentType);
            }
            else if constexpr (std::is_same_v<T, SuccessJsonCb>)
            {
                QJsonDocument json(QJsonDocument::fromJson(data));
                emit requestSuccessJson(std::move(json), std::move(cb));
            }
            else
            {
                try {
                    QJsonDocument json(QJsonDocument::fromJson(data));
                    auto reply = FromJson<T>::ReplyType::fromJson(json.object());
                    emit (this->*FromJson<T>::sEmitFun)(std::move(reply), std::move(cb));
                } catch (ATProto::InvalidJsonException& e) {
                    qWarning() << e.msg();
                    emit requestInvalidJsonError(e.msg(), errorCb);
                }
            }
        },
        successCb
    );
}

bool NetworkThread::resendRequest(Request request, const CallbackType& successCb, const ErrorCb& errorCb)
{
    if (request.mResendCount >= MAX_RESEND)
    {
        qWarning() << "Maximum resends reached:" << request.mXrpcRequest.url();
        return false;
    }

    ++request.mResendCount;
    qDebug() << "Resend:" << request.mXrpcRequest.url() << "count:" << request.mResendCount;
    sendRequest(request, successCb, errorCb);
    return true;
}

bool NetworkThread::resendWithNewDpopNonce(Request request, const CallbackType& successCb, const ErrorCb& errorCb)
{
    Q_ASSERT(!mDpopPdsNonce.isEmpty());
    const QUrl requestUrl = request.mXrpcRequest.url();

    if (request.mDpopResendCount >= MAX_DPOP_RESEND)
    {
        qWarning() << "Max DPoP resends reached:" << requestUrl;
        return false;
    }

    ++request.mDpopResendCount;
    qDebug() << "DPoP resend:" << requestUrl << "count:" << request.mDpopResendCount;
    const QString dpopProof = mDpopKey.buildPdsDPoPProof(
        request.mIsPost ? "POST" : "GET", requestUrl.toString(), request.mAccessJwt, mDpopPdsNonce);
    request.mXrpcRequest.setRawHeader("DPoP", dpopProof.toUtf8());

    sendRequest(request, successCb, errorCb);
    return true;
}

bool NetworkThread::mustResend(QNetworkReply::NetworkError error) const
{
    switch (error)
    {
    case QNetworkReply::NoError: // Unknown error seems to happen sometimes since Qt6.9.2
        qWarning() << "Retry on unknown error";
    case QNetworkReply::ContentReSendError:
    case QNetworkReply::OperationCanceledError: // Timeout
    case QNetworkReply::RemoteHostClosedError:
    case QNetworkReply::TimeoutError:
        return true;
    default:
        break;
    }

    return false;
}

QUrl NetworkThread::buildUrl(const QString& service, const QString& pds) const
{
    Q_ASSERT(!service.isEmpty());

    if (!pds.isEmpty())
        return QUrl(pds + "/xrpc/" + service);

    if (service.startsWith("app.bsky.video."))
    {
        return QUrl(mVideoHost + "/xrpc/" + service);
    }
    else
    {
        Q_ASSERT(!mPDS.isEmpty());
        return QUrl(mPDS + "/xrpc/" + service);
    }
}

QUrl NetworkThread::buildUrl(const QString& service, const Params& params, const QString& pds) const
{
    QUrl url = buildUrl(service, pds);
    QUrlQuery query;

    if (!params.isEmpty())
    {
        for (const auto& kv : params)
            query.addQueryItem(kv.first, QUrl::toPercentEncoding(kv.second));

        url.setQuery(query);
    }

    return url;
}

void NetworkThread::setUserAgentHeader(QNetworkRequest& request) const
{
    if (!mUserAgent.isEmpty())
        request.setHeader(QNetworkRequest::UserAgentHeader, mUserAgent);
}

void NetworkThread::setAuthorization(Request& request, const QString& accessJwt) const
{
    if (mOAuth)
    {
        const QUrl url = request.mXrpcRequest.url();
        const QString dpopProof = mDpopKey.buildPdsDPoPProof(
            request.mIsPost ? "POST" : "GET", url.toString(), accessJwt, mDpopPdsNonce);

        QString auth = QString("DPoP %1").arg(accessJwt);
        request.mXrpcRequest.setRawHeader("Authorization", auth.toUtf8());
        request.mXrpcRequest.setRawHeader("DPoP", dpopProof.toUtf8());
    }
    else
    {
        QString auth = QString("Bearer %1").arg(accessJwt);
        request.mXrpcRequest.setRawHeader("Authorization", auth.toUtf8());
    }

    request.mAccessJwt = accessJwt;
}

void NetworkThread::setRawHeaders(QNetworkRequest& request, const Params& params) const
{
    for (const auto& p : params)
    {
        qDebug() << p.first << ":" << p.second;
        request.setRawHeader(p.first.toUtf8(), p.second.toUtf8());
    }
}

void NetworkThread::enableOAuth(const QString& user, const QString& clientId, const QString& redirectUrl)
{
    qDebug() << "Enable OAuth:" << user << "clientId:" << clientId << "redirectUrl:" << redirectUrl;
    mDpopKey = ATProto::JsonWebKey::generateDPoPKey(user);
    mOAuth = std::make_unique<ATProto::OAuth>(user, mPDS, clientId, redirectUrl, &mDpopKey, mNetwork, this);
    mOAuth->setUserAgent(mUserAgent);
    mDpopPdsNonce.clear();
}

void NetworkThread::disableOAuth()
{
    qDebug() << "Disable OAuth";
    mOAuth.reset();
}

void NetworkThread::oauthLogin(const QString& user, const QString& clientId,
                               const QString& redirectUrl, const QStringList& scope,
                               const OAuthLoginSuccessCb& successCb, const OAuthErrorCb& errorCb)
{
    qDebug() << "Login:" << user << "clientId:" << clientId << "redirectUrl:" << redirectUrl << "scope:" << scope;
    enableOAuth(user, clientId, redirectUrl);

    mOAuth->login(scope,
        [this, successCb](QString state, QString issuer, QUrl redirectUrl){
            qDebug() << "Login state:" << state << "issuer:" << issuer << "redirect:" << redirectUrl;
            mOAuthState = state;
            mOAuthIssuer = issuer;
            emit oauthLoginRedirect(std::move(redirectUrl), successCb);
        },
        [this, errorCb](QString code, QString msg){
            qWarning() << "Login error:" << code << msg;
            emit oauthLoginFailed(std::move(code), std::move(msg), errorCb);
        });
}

void NetworkThread::oauthRequestInitialToken(const QUrl& url,
                                             const OAuthInitalTokenSuccessCb& successCb, const OAuthErrorCb& errorCb)
{
    qDebug() << "Request initial token:" << url;
    const QUrlQuery query(url.query());
    const QString state = query.queryItemValue("state", QUrl::FullyDecoded);

    if (state != mOAuthState)
    {
        qWarning() << "Unexpected state:" << state << "expected:" << mOAuthState;
        emit oauthRequestInitialTokenFailed(ATProto::OAuth::ERROR_SERVER_ERROR, "Unexpected state", errorCb);
        return;
    }

    const QString error = query.queryItemValue("error", QUrl::FullyDecoded);

    if (!error.isEmpty())
    {
        // Spaces seem to be encoded as '+'s
        const QString errorDescription = query.queryItemValue("error_description", QUrl::FullyDecoded).replace('+', ' ');
        qWarning() << "Error:" << error << "description:" << errorDescription;
        emit oauthRefreshTokenFailed(error, errorDescription, errorCb);
        return;
    }

    const QString issuer = query.queryItemValue("iss", QUrl::FullyDecoded);
    const QString code = query.queryItemValue("code", QUrl::FullyDecoded);
    qDebug() << "state:" << state << "issuer:" << issuer << "code:" << code;

    if (issuer != mOAuthIssuer)
    {
        qWarning() << "Unexpected issuer:" << issuer << "expected:" << mOAuthIssuer;
        emit oauthRequestInitialTokenFailed(ATProto::OAuth::ERROR_SERVER_ERROR, "Unexpected issuer", errorCb);
        return;
    }

    mOAuth->initialTokenRequest(code,
        [this, successCb](QString did, QString scope, QString accessToken, QString refreshToken){
            qDebug() << "Token sucess did:" << did << "scope:" << scope << "access:" << accessToken << "refresh:" << refreshToken;
            oauthRequestInitialTokenSuccess(did, scope, accessToken, refreshToken, successCb);
        },
        [this, errorCb](QString code, QString error){
            qWarning() << "Token error:" << code << error;
            emit oauthRequestInitialTokenFailed(code, error, errorCb);
        });
}

void NetworkThread::oauthRefreshToken(const QString& refreshToken,
                                      const OAuthRefreshTokenSuccessCb& successCb, const OAuthErrorCb& errorCb)
{
    qDebug() << "Refresh token";
    mOAuth->refreshTokenRequest(refreshToken,
        [this, successCb](QString newAccessToken, QString newRefreshToken){
            qDebug() << "Token refreshed access:" << newAccessToken << "refresh:" << newRefreshToken;
            emit oauthRefreshTokenSucces(newAccessToken, newRefreshToken, successCb);
        },
        [this, errorCb](QString code, QString error){
            qWarning() << "Token refresh error:" << code << error;
            emit oauthRefreshTokenFailed(code, error, errorCb);
        });
}

void NetworkThread::oauthResumeSession(const QString& user, const QString& clientId,
                                       const QString& redirectUrl, const QString& refreshToken,
                                       const OAuthRefreshTokenSuccessCb& successCb, const OAuthErrorCb& errorCb)
{
    // TODO: need redirectUrl?
    qDebug() << "Resume session:" << user << "clientId:" << clientId << "redirectUrl:" << redirectUrl;
    enableOAuth(user, clientId, redirectUrl);

    mOAuth->resumeSession(refreshToken,
        [this, successCb](QString newAccessToken, QString newRefreshToken){
            qDebug() << "Resumed session access:" << newAccessToken << "refresh:" << newRefreshToken;
            emit oauthRefreshTokenSucces(newAccessToken, newRefreshToken, successCb);
        },
        [this, errorCb](QString code, QString error){
            qWarning() << "Resume session error:" << code << error;
            emit oauthRefreshTokenFailed(code, error, errorCb);
        });
}

void NetworkThread::oauthLogout(const QString& accessToken, const QString& refreshToken,
                                const OAuthLogoutSuccessCb& successCb)
{
    qDebug() << "Logout";
    mOAuth->logout(accessToken, refreshToken,
        [this, successCb]{
            qDebug() << "Logout succces";
            oauthCleanup();
            emit oauthLoggedOut(successCb);
        },
        [this, successCb](QString code, QString error){
            qWarning() << "Logout error:" << code << error;
            oauthCleanup();
            emit oauthLoggedOut(successCb);
        });
}

void NetworkThread::oauthCleanup()
{
#if defined(Q_OS_ANDROID) && defined(USE_ANDROID_KEYSTORE)
    const QString alias = mDpopKey.getAlias();

    if (JsonWebKey::deleteKey(alias))
        qDebug() << "Deleted key:" << alias;
#endif
    mDpopKey = {};
    mOAuthState.clear();
    mOAuthIssuer.clear();
    mDpopPdsNonce.clear();
}

}
