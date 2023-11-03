// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "xrpc_client.h"
#include <QSslSocket>
#include <QUrlQuery>

namespace Xrpc {

Client::Client(const QString& host) :
    mHost(host)
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

    QNetworkRequest request(buildUrl(service));

    if (!accessJwt.isNull())
        setAuthorization(request, accessJwt);

    request.setHeader(QNetworkRequest::ContentTypeHeader, mimeType);
    QNetworkReply* reply = mNetwork.post(request, data);

    // In case of an error multiple callbacks may fire. First errorOcccured() and then probably finished()
    // The latter call is not guaranteed however. We must only call errorCb once!
    auto errorHandled = std::make_shared<bool>(false);

    connect(reply, &QNetworkReply::finished, this,
            [this, reply, successCb, errorCb, errorHandled]{ replyFinished(reply, successCb, errorCb, errorHandled); });
    connect(reply, &QNetworkReply::errorOccurred, this,
            [this, reply, errorCb, errorHandled](auto errorCode){ networkError(reply, errorCode, errorCb, errorHandled); });
    connect(reply, &QNetworkReply::sslErrors, this,
            [this, reply, errorCb, errorHandled](const QList<QSslError>& errors){ sslErrors(reply, errors, errorCb, errorHandled); });
}

void Client::get(const QString& service, const Params& params,
                 const SuccessCb& successCb, const ErrorCb& errorCb, const QString& accessJwt)
{
    Q_ASSERT(!service.isEmpty());
    Q_ASSERT(successCb);
    Q_ASSERT(errorCb);

    QNetworkRequest request(buildUrl(service, params));

    if (!accessJwt.isNull())
        setAuthorization(request, accessJwt);

    QNetworkReply* reply = mNetwork.get(request);

    // In case of an error multiple callbacks may fire. First errorOcccured() and then probably finished()
    // The latter call is not guaranteed however. We must only call errorCb once!
    auto errorHandled = std::make_shared<bool>(false);

    connect(reply, &QNetworkReply::finished, this,
            [this, reply, successCb, errorCb, errorHandled]{ replyFinished(reply, successCb, errorCb, errorHandled); });
    connect(reply, &QNetworkReply::errorOccurred, this,
            [this, reply, errorCb, errorHandled](auto errorCode){ networkError(reply, errorCode, errorCb, errorHandled); });
    connect(reply, &QNetworkReply::sslErrors, this,
            [this, reply, errorCb, errorHandled](const QList<QSslError>& errors){ sslErrors(reply, errors, errorCb, errorHandled); });
}

QUrl Client::buildUrl(const QString& service) const
{
    Q_ASSERT(!service.isEmpty());

    if (service == "legacy.searchPosts")
        return QUrl(QString("https://search.") + mHost + "/search/posts");
    if (service == "legacy.searchActors")
        return QUrl(QString("https://search.") + mHost + "/search/profiles");

    return QUrl(QString("https://") + mHost + "/xrpc/" + service);
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

void Client::replyFinished(QNetworkReply* reply, const SuccessCb& successCb, const ErrorCb& errorCb, std::shared_ptr<bool> errorHandled)
{
    Q_ASSERT(reply);
    qDebug() << "Reply:" << reply->error();
    const auto data = reply->readAll();
    const QJsonDocument json(QJsonDocument::fromJson(data));

    if (reply->error() == QNetworkReply::NoError)
    {
        successCb(json);
    }
    else if (!*errorHandled)
    {
        qDebug() << data;
        *errorHandled = true;
        errorCb(reply->errorString(), json);
    }
    else
    {
        qDebug() << "Error already handled";
    }
}

void Client::networkError(QNetworkReply* reply, QNetworkReply::NetworkError errorCode, const ErrorCb& errorCb, std::shared_ptr<bool> errorHandled)
{
    Q_ASSERT(reply);
    const auto errorMsg = reply->errorString();
    qInfo() << "Network error:" << errorCode << errorMsg;
    const auto data = reply->readAll();
    const QJsonDocument json(QJsonDocument::fromJson(data));

    if (!*errorHandled)
    {
        *errorHandled = true;
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
    // TODO: error handling
    qWarning() << "SSL errors:" << errors;

    if (!*errorHandled)
    {
        *errorHandled = true;
        errorCb("SSL error", {});
    }
    else
    {
        qDebug() << "Error already handled";
    }
}

}
