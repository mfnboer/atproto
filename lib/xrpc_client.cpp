// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "xrpc_client.h"
#include <QNetworkReply>
#include <QSslSocket>
#include <QUrlQuery>

namespace Xrpc {

Client::Client(const QString& host) :
    mHost(host)
{
    qDebug() << "Device supports OpenSSL: " << QSslSocket::supportsSsl();
    qDebug() << "OpenSSL lib:" << QSslSocket::sslLibraryVersionString();
    qDebug() << "OpenSSL lib build:" << QSslSocket::sslLibraryBuildVersionString();
}

Client::~Client()
{
    for (auto* reply : mReplies)
        delete reply;
}

void Client::post(const QString& service, const QJsonDocument& json,
                  const SuccessCb& successCb, const ErrorCb& errorCb, const QString& accessJwt)
{
    Q_ASSERT(!service.isEmpty());
    Q_ASSERT(successCb);
    Q_ASSERT(errorCb);

    QNetworkRequest request(buildUrl(service));

    if (!accessJwt.isNull())
        setAuthorization(request, accessJwt);

    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    const QByteArray data(json.toJson(QJsonDocument::Compact));
    QNetworkReply* reply = mNetwork.post(request, data);
    mReplies.insert(reply);

    connect(reply, &QNetworkReply::finished, this,
            [this, reply, successCb, errorCb]{ replyFinished(reply, successCb, errorCb); });
    connect(reply, &QNetworkReply::sslErrors, this,
            [this, reply, errorCb](const QList<QSslError>& errors){ sslErrors(reply, errors, errorCb); });
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
    mReplies.insert(reply);

    connect(reply, &QNetworkReply::finished, this,
            [this, reply, successCb, errorCb]{ replyFinished(reply, successCb, errorCb); });
    connect(reply, &QNetworkReply::sslErrors, this,
            [this, reply, errorCb](const QList<QSslError>& errors){ sslErrors(reply, errors, errorCb); });
}

void Client::removeReply(QNetworkReply* reply)
{
    mReplies.erase(reply);
    delete reply;
}

QUrl Client::buildUrl(const QString& service) const
{
    Q_ASSERT(!service.isEmpty());
    return QUrl(QString("https://") + mHost + "/xrpc/" + service);
}

QUrl Client::buildUrl(const QString& service, const Params& params) const
{
    QUrl url = buildUrl(service);
    QUrlQuery query;

    for (const auto& kv : params)
        query.addQueryItem(kv.first, QUrl::toPercentEncoding(kv.second));

    url.setQuery(query);
    return url;
}

void Client::setAuthorization(QNetworkRequest& request, const QString& accessJwt) const
{
    QString auth = QString("Bearer %1").arg(accessJwt);
    request.setRawHeader("Authorization", auth.toUtf8());
}

void Client::replyFinished(QNetworkReply* reply, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    Q_ASSERT(reply);
    qDebug() << "Reply:" << reply->error();
    const auto data = reply->readAll();
    const QJsonDocument json(QJsonDocument::fromJson(data));

    if (reply->error() == QNetworkReply::NoError)
    {
        successCb(json);
    }
    else
    {
        const auto data = reply->readAll();
        qDebug() << data;
        errorCb(reply->errorString(), json);
    }

    removeReply(reply);
}

void Client::sslErrors(QNetworkReply* reply, const QList<QSslError>& errors, const ErrorCb& errorCb)
{
    // TODO: error handling
    qWarning() << "SSL errors:" << errors;
    errorCb("SSL error", {});
    removeReply(reply);
}

}
