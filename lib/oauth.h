// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "json_web_key.h"
#include "presence.h"
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QUrlQuery>

namespace ATProto {

class OAuth : public QObject, public Presence
{
public:
    using AuthServerSuccessCb = std::function<void(QJsonDocument resp)>;
    using ParSuccessCb = std::function<void(QString pkceVerifier, QString state, QJsonDocument resp)>;
    using ErrorCb = std::function<void(int code, QString msg)>;

    explicit OAuth(QObject* parent = nullptr);
    void setUserAgent(const QString& userAgent) { mUserAgent = userAgent; }
    void sendParAuthRequest(const QString& parUrl, const QString& loginHint, const QString& clientId,
                            const QString& redirectUrl, const QString& scope, const JsonWebKey& dpopPrivateJwk,
                            const ParSuccessCb& successCb, const ErrorCb& errorCb);

private:
    void authServerPost(const JsonWebKey& dpopPrivateJwk, const QString& dpopAuthServerNonce,
                        const QString& postUrl, const QUrlQuery& postData,
                        const AuthServerSuccessCb& successCb, const ErrorCb& errorCb);

    QString createPkceCodeChallenge(const QString& verifier) const;

    void setUserAgentHeader(QNetworkRequest& request) const;

    std::unique_ptr<QNetworkAccessManager> mNetwork;
    QString mUserAgent;
};

}
