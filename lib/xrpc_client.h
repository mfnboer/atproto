// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <unordered_set>

namespace Xrpc {

class Client : public QObject
{
public:
    using ErrorCb = std::function<void(const QString& err, const QJsonDocument& json)>;
    using SuccessCb = std::function<void(const QJsonDocument& json)>;
    using Params = QList<QPair<QString, QString>>;

    explicit Client(const QString& host);
    ~Client();

    const QString& getHost() const { return mHost; }

    void post(const QString& service, const QJsonDocument& json,
              const SuccessCb& successCb, const ErrorCb& errorCb, const QString& accessJwt = {});
    void post(const QString& service, const QByteArray& data, const QString& mimeType,
              const SuccessCb& successCb, const ErrorCb& errorCb, const QString& accessJwt);
    void get(const QString& service, const Params& params,
             const SuccessCb& successCb, const ErrorCb& errorCb, const QString& accessJwt = {});

private:
    QUrl buildUrl(const QString& service) const;
    QUrl buildUrl(const QString& service, const Params& params) const;
    void setAuthorization(QNetworkRequest& request, const QString& accessJwt) const;
    void replyFinished(QNetworkReply* reply, const SuccessCb& successCb, const ErrorCb& errorCb);
    void networkError(QNetworkReply* reply, QNetworkReply::NetworkError errorCode, const ErrorCb& errorCb);
    void sslErrors(QNetworkReply* reply, const QList<QSslError>& errors, const ErrorCb& errorCb);

    QNetworkAccessManager mNetwork;
    QString mHost;
};

}
