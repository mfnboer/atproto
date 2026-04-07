// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "lexicon/plc_directory.h"
#include "network_client.h"
#include "xjson.h"
#include <QCache>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace ATProto {

struct PlcRequest
{
    QNetworkRequest mNetworkRequest;
    int mResendCount = 0;
    bool mIsPost = false;
    QByteArray mPostData;
};

using PlcErrorCb = std::function<void(int errorCode, const QString& errorMsg)>;
using PlcSuccessJsonCb = std::function<void(const QJsonDocument& json)>;

class PlcDirectoryClient : public NetworkClient<PlcRequest, PlcSuccessJsonCb, PlcErrorCb>
{
public:
    using ErrorCb = PlcErrorCb;
    using SuccessJsonCb = PlcSuccessJsonCb;
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
    using Request = PlcRequest;

    virtual void replyFinished(const Request& request, QNetworkReply* reply,
                       const SuccessJsonCb& successCb, const ErrorCb& errorCb,
                       std::shared_ptr<bool> errorHandled) override;

    virtual void networkError(const Request& request, QNetworkReply* reply, QNetworkReply::NetworkError errorCode,
                      const SuccessJsonCb& successCb, const ErrorCb& errorCb,
                      std::shared_ptr<bool> errorHandled) override;


    void invalidJsonError(InvalidJsonException& e, const ErrorCb& cb);
    void invokeErrorCb(const QJsonDocument& jsonDoc, QNetworkReply* reply, QNetworkReply::NetworkError errorCode, const ErrorCb& errorCb);

    QString mHost;
    QCache<QString, QDateTime> mFirstAppearanceCache; // DID -> datetime
    QCache<QString, QString> mPdsCache; // DID -> PDS
};

}
