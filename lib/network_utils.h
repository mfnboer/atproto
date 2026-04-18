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
    static bool isDpopNonceError(QNetworkReply* reply, const QByteArray& data);
    static bool hasDpopNonce(QNetworkReply* reply);
    static QString getDpopNonce(QNetworkReply* reply);
};

}
