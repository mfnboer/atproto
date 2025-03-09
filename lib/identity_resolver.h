// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace ATProto {

class IdentityResolver : public QObject
{
public:
    using Ptr = std::unique_ptr<IdentityResolver>;
    using ErrorCb = std::function<void(const QString& error)>;
    using SuccessCb = std::function<void(const QString& did)>;

    IdentityResolver();
    void resolveHandle(const QString& handle, const SuccessCb& successCb, const ErrorCb& errorCb);

private:
    QString getDnsLookupName(const QString& handle) const;
    QUrl getDohUrl(const QString& handle) const;
    void handleDnsResult(const QString& handle, const SuccessCb& successCb, const ErrorCb& errorCb);
    void handleDohResponse(QNetworkReply* reply, const QString& handle, const SuccessCb& successCb, const ErrorCb& errorCb);
    QUrl getHttpUrl(const QString& handle) const;
    void httpGetDid(const QString& handle, const SuccessCb& successCb, const ErrorCb& errorCb);
    void handleHttpResponse(QNetworkReply* reply, const QString& handle, const SuccessCb& successCb, const ErrorCb& errorCb);

    // QDnsLookup not supported on Android
    // std::unique_ptr<QDnsLookup> mDns;

    QNetworkAccessManager mNetwork;
};

}
