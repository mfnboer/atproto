// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "lexicon/plc_directory.h"
#include "presence.h"
#include "xjson.h"
#include <QCache>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace ATProto {

class PlcDirectoryClient : public QObject, public Presence
{
public:
    using ErrorCb = std::function<void(int errorCode, const QString& errorMsg)>;
    using SuccessJsonCb = std::function<void(const QJsonDocument& json)>;
    using PdsSuccessCb = std::function<void(const QString& pds)>;
    using AuditLogSuccessCb = std::function<void(PlcAuditLog::SharedPtr)>;
    using FirstAppearanceSuccessCb = std::function<void(QDateTime)>;

    static constexpr const char* PLC_DIRECTORY_HOST = "plc.directory";

    explicit PlcDirectoryClient(QNetworkAccessManager* network, const QString host = PLC_DIRECTORY_HOST, QObject* parent = nullptr);

    void getPds(const QString& did, const PdsSuccessCb& successCb, const ErrorCb& errorCb);
    void getAuditLog(const QString& did, const AuditLogSuccessCb& successCb, const ErrorCb& errorCb);
    void getFirstAppearance(const QString& did, const FirstAppearanceSuccessCb& successCb, const ErrorCb& errorCb);

    void invalidatePdsCache(const QString& did);

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

    QNetworkAccessManager* mNetwork;
    QString mHost;
    QCache<QString, QDateTime> mFirstAppearanceCache; // DID -> datetime
    QCache<QString, QString> mPdsCache; // DID -> PDS
};

}
