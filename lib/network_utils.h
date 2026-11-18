// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <QNetworkReply>
#include <QUrl>

namespace ATProto {

class NetworkUtils
{
public:
    static bool isSafeUrl(const QUrl url);
    static bool isInvalidTokenError(const QByteArray& replyData);
    static bool isInvalidTokenError(const QJsonDocument& replyJson);
    static bool isDpopNonceError(QNetworkReply* reply, const QByteArray& data);
    static bool isDpopNonceError(QNetworkReply* reply, const QJsonDocument& replyJson);
    static bool hasDpopNonce(QNetworkReply* reply);
    static QString getDpopNonce(QNetworkReply* reply);
    static bool hasError(const QByteArray& replyData,
                         const std::function<bool(const QString& error)>& errorCheck =
                                [](const QString& error){ return !error.isEmpty(); });
    static bool hasError(const QJsonDocument& replyJson,
                         const std::function<bool(const QString& error)>& errorCheck =
                         [](const QString& error){ return !error.isEmpty(); });

};

}
