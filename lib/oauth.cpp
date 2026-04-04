// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "oauth.h"
#include <QCryptographicHash>

namespace ATProto {

OAuth::OAuth(QObject* parent) :
    QObject(parent),
    mNetwork(new QNetworkAccessManager)
{
    mNetwork->setAutoDeleteReplies(true);
    mNetwork->setTransferTimeout(10000);
}

void OAuth::authServerPost(const JsonWebKey& dpopPrivateJwk, const QString& dpopAuthServerNonce,
                    const QString& postUrl, const QUrlQuery& postData,
                    const AuthServerSuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Post:" << postUrl;
    const QString dpopProof = dpopPrivateJwk.buildDPoPProof("POST", postUrl, dpopAuthServerNonce);

    // TODO: hardened http, SSRF

    QNetworkRequest request(postUrl);
    setUserAgentHeader(request);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("DPoP", dpopProof.toUtf8());
    QNetworkReply* reply = mNetwork->post(request, postData.toString().toUtf8());

    connect(reply, &QNetworkReply::finished, this,
        [presence=getPresence(), reply, successCb, errorCb]{
            if (!presence)
                return;

            const auto errorCode = reply->error();

            if (errorCode == QNetworkReply::NoError)
            {
                const auto data = reply->readAll();
                const QJsonDocument json(QJsonDocument::fromJson(data));
                successCb(json);
            }
            else
            {
                // TODO: Resend?
                int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
                QString reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
                errorCb(status, reason);
            }
    });
}

void OAuth::sendParAuthRequest(const QString& parUrl, const QString& loginHint, const QString& clientId,
                        const QString& redirectUrl, const QString& scope, const JsonWebKey& dpopPrivateJwk,
                        const ParSuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Send PAR:" << parUrl << "loginHint:" << loginHint << "clientId:" << clientId << "redirectUrl:" << redirectUrl << "scope:" << scope;
    const QString state = JsonWebKey::generateToken();
    const QString pkceVerifier = JsonWebKey::generateToken(48);
    const QString codeChallenge = createPkceCodeChallenge(pkceVerifier);

    QUrlQuery parBody;
    parBody.addQueryItem("client_id", clientId);
    parBody.addQueryItem("reponse_type", "code");
    parBody.addQueryItem("code_challend", codeChallenge);
    parBody.addQueryItem("code_challenge_method", "S256");
    parBody.addQueryItem("state", state);
    parBody.addQueryItem("scope", scope);

    if (!loginHint.isEmpty())
        parBody.addQueryItem("login_hint", loginHint);

    authServerPost(dpopPrivateJwk, "", parUrl, parBody,
        [presence=getPresence(), pkceVerifier, state, successCb](QJsonDocument resp){
            if (!presence)
                return;

            qDebug() << "PAR success, PKCE:" << pkceVerifier << "state:" << state << "resp:" << resp;
            successCb(pkceVerifier, state, resp);
        },
        [errorCb](int errorCode, QString errorMsg){
            qDebug() << "PAR failed:" << errorCode << errorMsg;
            errorCb(errorCode, errorMsg);
        }
    );
}

QString OAuth::createPkceCodeChallenge(const QString& verifier) const
{
    const auto hash = QCryptographicHash::hash(verifier.toUtf8(), QCryptographicHash::Sha256);
    return hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
}

void OAuth::setUserAgentHeader(QNetworkRequest& request) const
{
    if (!mUserAgent.isEmpty())
        request.setHeader(QNetworkRequest::UserAgentHeader, mUserAgent);
}

}
