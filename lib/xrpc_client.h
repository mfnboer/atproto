// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <unordered_set>

namespace Xrpc {

class Client : public QObject
{
public:
    using ErrorCb = std::function<void(const QString& err, const QJsonDocument& json)>;
    using SuccessCb = std::function<void(const QJsonDocument& json)>;
    using Params = std::initializer_list<QPair<QString, QString>>;

    explicit Client(const QString& host);
    ~Client();

    void post(const QString& service, const QJsonDocument& json,
              const SuccessCb& successCb, const ErrorCb& errorCb, const QString& accessJwt = {});
    void get(const QString& service, const Params& params,
             const SuccessCb& successCb, const ErrorCb& errorCb, const QString& accessJwt = {});

private:
    QUrl buildUrl(const QString& service) const;
    void setAuthorization(QNetworkRequest& request, const QString& accessJwt) const;
    void removeReply(const QNetworkReply* reply);
    void replyFinished(QNetworkReply* reply, const SuccessCb& successCb, const ErrorCb& errorCb);
    void sslErrors(const QNetworkReply* reply, const QList<QSslError>& errors, const ErrorCb& errorCb);

    QNetworkAccessManager mNetwork;
    QString mHost;
    std::unordered_set<QNetworkReply*> mReplies;
};

}
