// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "xrpc_client.h"
#include "lexicon/lexicon.h"
#include <QSslSocket>
#include <QUrlQuery>

namespace Xrpc {

constexpr int MAX_RESEND = 4;

Client::Client(const QString& host) :
    mHost(host),
    mPDS("https://" + host)
{
    qDebug() << "Device supports OpenSSL: " << QSslSocket::supportsSsl();
    qDebug() << "OpenSSL lib:" << QSslSocket::sslLibraryVersionString();
    qDebug() << "OpenSSL lib build:" << QSslSocket::sslLibraryBuildVersionString();
    mNetwork.setAutoDeleteReplies(true);
    mNetwork.setTransferTimeout(15000);
}

Client::~Client()
{
    qDebug() << "Destroy client";
}

void Client::setPDS(const QString& pds)
{
    if (pds.startsWith("http"))
        mPDS = pds;
    else
        mPDS = "https://" + pds;

    qDebug() << "PDS:" << mPDS;
}

void Client::setPDSFromSession(const ATProto::ComATProtoServer::Session& session)
{
    const auto pds = session.getPDS();

    if (pds)
        setPDS(*pds);
    else
        setPDS("https://" + mHost);
}

void Client::post(const QString& service, const QJsonDocument& json, const Params& rawHeaders,
                  const SuccessJsonCb& successCb, const ErrorCb& errorCb, const QString& accessJwt)
{
    const QByteArray data(json.toJson(QJsonDocument::Compact));
    post(service, data, "application/json", rawHeaders, successCb, errorCb, accessJwt);
}

static bool isEmpty(const Client::DataType& data)
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

void Client::post(const QString& service, const DataType& data, const QString& mimeType, const Params& rawHeaders,
                  const SuccessJsonCb& successCb, const ErrorCb& errorCb, const QString& accessJwt)
{
    Q_ASSERT(!service.isEmpty());
    Q_ASSERT(successCb);
    Q_ASSERT(errorCb);

    Request request;
    request.mIsPost = true;
    request.mXrpcRequest = QNetworkRequest(buildUrl(service));
    setUserAgent(request.mXrpcRequest);

    if (!accessJwt.isNull())
        setAuthorization(request.mXrpcRequest, accessJwt);

    // Setting Content-Type header when no body is present causes this error on some
    // PDS' as of 23-6-2024
    if (!isEmpty(data))
        request.mXrpcRequest.setHeader(QNetworkRequest::ContentTypeHeader, mimeType);

    setRawHeaders(request.mXrpcRequest, rawHeaders);
    request.mData = data;
    sendRequest(request, successCb, errorCb);
}

void Client::get(const QString& service, const Params& params, const Params& rawHeaders,
                 const SuccessJsonCb& successCb, const ErrorCb& errorCb, const QString& accessJwt)
{
    getImpl(service, params, rawHeaders, successCb, errorCb, accessJwt);
}

void Client::get(const QString& service, const Params& params, const Params& rawHeaders,
                 const SuccessBytesCb& successCb, const ErrorCb& errorCb, const QString& accessJwt)
{
    getImpl(service, params, rawHeaders, successCb, errorCb, accessJwt);
}

template<typename Callback>
void Client::getImpl(const QString& service, const Params& params, const Params& rawHeaders,
                     const Callback& successCb, const ErrorCb& errorCb, const QString& accessJwt)
{
    Q_ASSERT(!service.isEmpty());
    Q_ASSERT(successCb);
    Q_ASSERT(errorCb);

    Request request;
    request.mIsPost = false;
    request.mXrpcRequest =  QNetworkRequest(buildUrl(service, params));
    setUserAgent(request.mXrpcRequest);

    if (!accessJwt.isNull())
        setAuthorization(request.mXrpcRequest, accessJwt);

    setRawHeaders(request.mXrpcRequest, rawHeaders);
    sendRequest(request, successCb, errorCb);
}

QUrl Client::buildUrl(const QString& service) const
{
    Q_ASSERT(!service.isEmpty());

    if (service.startsWith("app.bsky.video."))
        return QUrl("https://video.bsky.app/xrpc/" + service);
    else
        return QUrl(mPDS + "/xrpc/" + service);
}

QUrl Client::buildUrl(const QString& service, const Params& params) const
{
    QUrl url = buildUrl(service);
    QUrlQuery query;

    if (!params.isEmpty())
    {
        for (const auto& kv : params)
            query.addQueryItem(kv.first, QUrl::toPercentEncoding(kv.second));

        url.setQuery(query);
    }

    return url;
}

void Client::setUserAgent(QNetworkRequest& request) const
{
    if (!mUserAgent.isEmpty())
        request.setHeader(QNetworkRequest::UserAgentHeader, mUserAgent);
}

void Client::setAuthorization(QNetworkRequest& request, const QString& accessJwt) const
{
    QString auth = QString("Bearer %1").arg(accessJwt);
    request.setRawHeader("Authorization", auth.toUtf8());
}

void Client::setRawHeaders(QNetworkRequest& request, const Params& params) const
{
    for (const auto& p : params)
    {
        qDebug() << p.first << ":" << p.second;
        request.setRawHeader(p.first.toUtf8(), p.second.toUtf8());
    }
}

static void invokeCallback(const Client::SuccessBytesCb& successCb, const QByteArray& data, const QString& contentType)
{
    successCb(data, contentType);
}

static void invokeCallback(const Client::SuccessJsonCb& successCb, const QByteArray& data, const QString&)
{
    const QJsonDocument json(QJsonDocument::fromJson(data));
    successCb(json);
}

template<typename Callback>
void Client::replyFinished(const Request& request, QNetworkReply* reply,
                           const Callback& successCb, const ErrorCb& errorCb,
                           std::shared_ptr<bool> errorHandled)
{
    Q_ASSERT(reply);
    const auto errorCode = reply->error();
    const QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString().trimmed();
    qDebug() << "Reply:" << errorCode << "content:" << contentType;
    const auto data = reply->readAll();

    if (errorCode == QNetworkReply::NoError)
    {
        invokeCallback(successCb, data, contentType);
    }
    else if (!*errorHandled)
    {
        *errorHandled = true;

        if (mustResend(errorCode))
        {
            if (resendRequest(request, successCb, errorCb))
                return;
        }

        const QJsonDocument json(QJsonDocument::fromJson(data));
        errorCb(reply->errorString(), json);
    }
    else
    {
        qDebug() << "Error already handled";
    }
}

template<typename Callback>
void Client::networkError(const Request& request, QNetworkReply* reply, QNetworkReply::NetworkError errorCode,
                          const Callback& successCb, const ErrorCb& errorCb, std::shared_ptr<bool> errorHandled)
{
    Q_ASSERT(reply);
    const auto errorMsg = reply->errorString();
    qInfo() << "Network error:" << errorCode << errorMsg;

    if (errorCode == QNetworkReply::OperationCanceledError)
    {
        reply->disconnect();
        errorCb(ATProto::ATProtoErrorMsg::XRPC_TIMEOUT, {});
        return;
    }

    const auto data = reply->readAll();

    if (!*errorHandled)
    {
        *errorHandled = true;

        if (mustResend(errorCode))
        {
            if (resendRequest(request, successCb, errorCb))
                return;
        }

        const QJsonDocument json(QJsonDocument::fromJson(data));
        errorCb(errorMsg, json);
    }
    else
    {
        qDebug() << "Error already handled";
    }
}

void Client::sslErrors(QNetworkReply* reply, const QList<QSslError>& errors, const ErrorCb& errorCb, std::shared_ptr<bool> errorHandled)
{
    Q_ASSERT(reply);
    qWarning() << "SSL errors:" << errors;

    if (!*errorHandled)
    {
        *errorHandled = true;
        QString msg = "SSL error";

        if (!errors.empty())
            msg.append(": ").append(errors.front().errorString());

        errorCb(msg, {});
    }
    else
    {
        qDebug() << "Error already handled";
    }
}

template<typename Callback>
void Client::sendRequest(const Request& request, const Callback& successCb, const ErrorCb& errorCb)
{
    qDebug() << request.mXrpcRequest.rawHeader("User-Agent");
    QNetworkReply* reply;

    if (request.mIsPost)
    {
        if (std::holds_alternative<QByteArray>(request.mData))
        {
            const auto& bytes = std::get<QByteArray>(request.mData);
            reply = mNetwork.post(request.mXrpcRequest, bytes);
        }
        else
        {
            auto* ioDevice = std::get<QIODevice*>(request.mData);
            reply = mNetwork.post(request.mXrpcRequest, ioDevice);
        }
    }
    else
    {
        reply = mNetwork.get(request.mXrpcRequest);
    }

    // In case of an error multiple callbacks may fire. First errorOcccured() and then probably finished()
    // The latter call is not guaranteed however. We must only call errorCb once!
    auto errorHandled = std::make_shared<bool>(false);

    connect(reply, &QNetworkReply::finished, this,
            [this, request, reply, successCb, errorCb, errorHandled]{ replyFinished(request, reply, successCb, errorCb, errorHandled); });
    connect(reply, &QNetworkReply::errorOccurred, this,
            [this, request, reply, successCb, errorCb, errorHandled](auto errorCode){ this->networkError(request, reply, errorCode, successCb, errorCb, errorHandled); });
    connect(reply, &QNetworkReply::sslErrors, this,
            [this, reply, errorCb, errorHandled](const QList<QSslError>& errors){ sslErrors(reply, errors, errorCb, errorHandled); });
}

template<typename Callback>
bool Client::resendRequest(Request request, const Callback& successCb, const ErrorCb& errorCb)
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

bool Client::mustResend(QNetworkReply::NetworkError error) const
{
    switch (error)
    {
    case QNetworkReply::ContentReSendError:
    case QNetworkReply::RemoteHostClosedError:
        return true;
    default:
        break;
    }

    return false;
}

}
