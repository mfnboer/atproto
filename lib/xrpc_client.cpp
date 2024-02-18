// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "xrpc_client.h"
#include <QSslSocket>
#include <QUrlQuery>

namespace Xrpc {

constexpr int MAX_RESEND = 4;

QNetworkAccessManager Client::sNetwork;

Client::Client(const QString& host) :
    mHost(host),
    mPDS("https://" + host)
{
    qDebug() << "Device supports OpenSSL: " << QSslSocket::supportsSsl();
    qDebug() << "OpenSSL lib:" << QSslSocket::sslLibraryVersionString();
    qDebug() << "OpenSSL lib build:" << QSslSocket::sslLibraryBuildVersionString();
    sNetwork.setAutoDeleteReplies(true);
    sNetwork.setTransferTimeout(15000);
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

void Client::post(const QString& service, const QJsonDocument& json,
                  const SuccessCb& successCb, const ErrorCb& errorCb, const QString& accessJwt)
{
    const QByteArray data(json.toJson(QJsonDocument::Compact));
    post(service, data, "application/json", successCb, errorCb, accessJwt);
}

void Client::post(const QString& service, const QByteArray& data, const QString& mimeType,
          const SuccessCb& successCb, const ErrorCb& errorCb, const QString& accessJwt)
{
    Q_ASSERT(!service.isEmpty());
    Q_ASSERT(successCb);
    Q_ASSERT(errorCb);

    Request request;
    request.mIsPost = true;
    request.mXrpcRequest = QNetworkRequest(buildUrl(service));

    if (!accessJwt.isNull())
        setAuthorization(request.mXrpcRequest, accessJwt);

    request.mXrpcRequest.setHeader(QNetworkRequest::ContentTypeHeader, mimeType);
    request.mData = data;
    sendRequest(request, successCb, errorCb);
}

void Client::get(const QString& service, const Params& params,
                 const SuccessCb& successCb, const ErrorCb& errorCb, const QString& accessJwt)
{
    Q_ASSERT(!service.isEmpty());
    Q_ASSERT(successCb);
    Q_ASSERT(errorCb);

    Request request;
    request.mIsPost = false;
    request.mXrpcRequest =  QNetworkRequest(buildUrl(service, params));

    if (!accessJwt.isNull())
        setAuthorization(request.mXrpcRequest, accessJwt);

    sendRequest(request, successCb, errorCb);
}

QUrl Client::buildUrl(const QString& service) const
{
    Q_ASSERT(!service.isEmpty());
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

void Client::setAuthorization(QNetworkRequest& request, const QString& accessJwt) const
{
    QString auth = QString("Bearer %1").arg(accessJwt);
    request.setRawHeader("Authorization", auth.toUtf8());
}

void Client::replyFinished(const Request& request, QNetworkReply* reply,
                           const SuccessCb& successCb, const ErrorCb& errorCb,
                           std::shared_ptr<bool> errorHandled)
{
    Q_ASSERT(reply);
    qDebug() << "Reply:" << reply->error();
    const auto data = reply->readAll();
    const QJsonDocument json(QJsonDocument::fromJson(data));
    const auto errorCode = reply->error();

    if (errorCode == QNetworkReply::NoError)
    {
        successCb(json);
    }
    else if (!*errorHandled)
    {
        *errorHandled = true;

        if (mustResend(errorCode))
        {
            if (resendRequest(request, successCb, errorCb))
                return;
        }

        qDebug() << data;
        errorCb(reply->errorString(), json);
    }
    else
    {
        qDebug() << "Error already handled";
    }
}

void Client::networkError(const Request& request, QNetworkReply* reply, QNetworkReply::NetworkError errorCode,
                          const SuccessCb& successCb, const ErrorCb& errorCb, std::shared_ptr<bool> errorHandled)
{
    Q_ASSERT(reply);
    const auto errorMsg = reply->errorString();
    qInfo() << "Network error:" << errorCode << errorMsg;

    if (errorCode == QNetworkReply::OperationCanceledError)
    {
        reply->disconnect();
        errorCb(errorMsg, {});
        return;
    }

    const auto data = reply->readAll();
    const QJsonDocument json(QJsonDocument::fromJson(data));

    if (!*errorHandled)
    {
        *errorHandled = true;

        if (mustResend(errorCode))
        {
            if (resendRequest(request, successCb, errorCb))
                return;
        }

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

void Client::sendRequest(const Request& request, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    QNetworkReply* reply;

    if (request.mIsPost)
        reply = sNetwork.post(request.mXrpcRequest, request.mData);
    else
        reply = sNetwork.get(request.mXrpcRequest);

    // In case of an error multiple callbacks may fire. First errorOcccured() and then probably finished()
    // The latter call is not guaranteed however. We must only call errorCb once!
    auto errorHandled = std::make_shared<bool>(false);

    connect(reply, &QNetworkReply::finished, this,
            [this, request, reply, successCb, errorCb, errorHandled]{ replyFinished(request, reply, successCb, errorCb, errorHandled); });
    connect(reply, &QNetworkReply::errorOccurred, this,
            [this, request, reply, successCb, errorCb, errorHandled](auto errorCode){ networkError(request, reply, errorCode, successCb, errorCb, errorHandled); });
    connect(reply, &QNetworkReply::sslErrors, this,
            [this, reply, errorCb, errorHandled](const QList<QSslError>& errors){ sslErrors(reply, errors, errorCb, errorHandled); });
}

bool Client::resendRequest(Request request, const SuccessCb& successCb, const ErrorCb& errorCb)
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
