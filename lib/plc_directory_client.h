// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "lexicon/plc_directory.h"
#include "xjson.h"
#include <QCache>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace ATProto {

class PlcDirectoryClient : public QObject
{
public:
    using ErrorCb = std::function<void(int errorCode, const QString& errorMsg)>;
    using SuccessJsonCb = std::function<void(const QJsonDocument& json)>;
    using AuditLogSuccessCb = std::function<void(PlcAuditLog::SharedPtr)>;
    using FirstAppearanceSuccessCb = std::function<void(QDateTime)>;

    explicit PlcDirectoryClient(const QString host = "plc.directory");

    void getAuditLog(const QString& did, const AuditLogSuccessCb& successCb, const ErrorCb& errorCb);
    void getFirstAppearance(const QString& did, const FirstAppearanceSuccessCb& successCb, const ErrorCb& errorCb);

private:
    struct Request
    {
        QNetworkRequest mPlcRequest;
        int mResendCount = 0;
    };

    void sendRequest(const Request& request, const SuccessJsonCb& successCb, const ErrorCb& errorCb);

    void replyFinished(const Request& request, QNetworkReply* reply,
                       const SuccessJsonCb& successCb, const ErrorCb& errorCb,
                       std::shared_ptr<bool> errorHandled);

    void networkError(const Request& request, QNetworkReply* reply, QNetworkReply::NetworkError errorCode,
                      const SuccessJsonCb& successCb, const ErrorCb& errorCb,
                      std::shared_ptr<bool> errorHandled);

    void sslErrors(QNetworkReply* reply, const QList<QSslError>& errors, const ErrorCb& errorCb, std::shared_ptr<bool> errorHandled);

    bool resendRequest(Request request, const SuccessJsonCb& successCb, const ErrorCb& errorCb);
    bool mustResend(QNetworkReply::NetworkError error) const;

    void invalidJsonError(InvalidJsonException& e, const ErrorCb& cb);
    void invokeErrorCb(const QJsonDocument& jsonDoc, QNetworkReply* reply, QNetworkReply::NetworkError errorCode, const ErrorCb& errorCb);

    QString mHost;
    QNetworkAccessManager mNetwork;
    QCache<QString, QDateTime> mFirstAppearanceCache; // did -> datetime
};

}
