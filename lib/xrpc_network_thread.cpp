// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "xrpc_network_thread.h"
#include "lexicon/lexicon.h"
#include <QSslSocket>

namespace Xrpc {

constexpr int MAX_RESEND = 4;

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


NetworkThread::NetworkThread(QObject* parent) :
    QThread(parent)
{
}

void NetworkThread::run()
{
    qDebug() << "XRPC network thread running:" << currentThreadId();
    mNetwork = new QNetworkAccessManager(this);
    mNetwork->setAutoDeleteReplies(true);
    exec();
}

void NetworkThread::postData(const QString& service, const DataType& data, const QString& mimeType, const Params& rawHeaders,
              const SuccessJsonCb& successCb, const ErrorCb& errorCb, const QString& accessJwt)
{
    Request request;
    request.mIsPost = true;
    request.mXrpcRequest = QNetworkRequest(buildUrl(service));
    setUserAgentHeader(request.mXrpcRequest);

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

void NetworkThread::postJson(const QString& service, const QJsonDocument& json, const Params& rawHeaders,
              const SuccessJsonCb& successCb, const ErrorCb& errorCb, const QString& accessJwt)
{
    const QByteArray data(json.toJson(QJsonDocument::Compact));
    postData(service, data, "application/json", rawHeaders, successCb, errorCb, accessJwt);
}

void NetworkThread::get(const QString& service, const Params& params, const Params& rawHeaders,
         const CallbackType& successCb, const ErrorCb& errorCb, const QString& accessJwt)
{
    Request request;
    request.mIsPost = false;
    request.mXrpcRequest =  QNetworkRequest(buildUrl(service, params));
    setUserAgentHeader(request.mXrpcRequest);

    if (!accessJwt.isNull())
        setAuthorization(request.mXrpcRequest, accessJwt);

    setRawHeaders(request.mXrpcRequest, rawHeaders);
    sendRequest(request, successCb, errorCb);
}

void NetworkThread::sendRequest(const Request& request, const CallbackType& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Thread:" << currentThreadId();
    QNetworkReply* reply;

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
    qDebug() << "Reply:" << errorCode << "content:" << contentType;
    auto data = reply->readAll();

    if (errorCode == QNetworkReply::NoError)
    {
        invokeCallback(std::move(successCb), std::move(data), contentType);
    }
    else if (!*errorHandled)
    {
        *errorHandled = true;

        if (mustResend(errorCode))
        {
            if (resendRequest(request, successCb, errorCb))
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

    if (errorCode == QNetworkReply::OperationCanceledError)
    {
        reply->disconnect();
        emit requestError(ATProto::ATProtoErrorMsg::XRPC_TIMEOUT, {}, errorCb);
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

void NetworkThread::invokeCallback(CallbackType successCb, QByteArray data, const QString& contentType)
{
    if (std::holds_alternative<SuccessBytesCb>(successCb))
    {
        const auto& cb = std::get<SuccessBytesCb>(successCb);
        emit requestSuccessBytes(std::move(data), std::move(cb), contentType);
    }
    else if (std::holds_alternative<SuccessJsonCb>(successCb))
    {
        const auto& cb = std::get<SuccessJsonCb>(successCb);
        QJsonDocument json(QJsonDocument::fromJson(data));
        emit requestSuccessJson(std::move(json), std::move(cb));
    }
    else
    {
        qWarning() << "Invalid callback type";
    }
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

bool NetworkThread::mustResend(QNetworkReply::NetworkError error) const
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

QUrl NetworkThread::buildUrl(const QString& service) const
{
    Q_ASSERT(!service.isEmpty());

    if (service.startsWith("app.bsky.video."))
    {
        return QUrl("https://video.bsky.app/xrpc/" + service);
    }
    else
    {
        Q_ASSERT(!mPDS.isEmpty());
        return QUrl(mPDS + "/xrpc/" + service);
    }
}

QUrl NetworkThread::buildUrl(const QString& service, const Params& params) const
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

void NetworkThread::setUserAgentHeader(QNetworkRequest& request) const
{
    if (!mUserAgent.isEmpty())
        request.setHeader(QNetworkRequest::UserAgentHeader, mUserAgent);
}

void NetworkThread::setAuthorization(QNetworkRequest& request, const QString& accessJwt) const
{
    QString auth = QString("Bearer %1").arg(accessJwt);
    request.setRawHeader("Authorization", auth.toUtf8());
}

void NetworkThread::setRawHeaders(QNetworkRequest& request, const Params& params) const
{
    for (const auto& p : params)
    {
        qDebug() << p.first << ":" << p.second;
        request.setRawHeader(p.first.toUtf8(), p.second.toUtf8());
    }
}


}
