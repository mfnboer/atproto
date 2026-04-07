// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "test.h"
#include <QHostAddress>

namespace ATProto {

constexpr char const* OAUTH_SCOPE = "atproto";
constexpr char const* REDIRECT_URI = "http://127.0.0.1:1970/oauth/callback";
constexpr int LISTEN_PORT = 1970;
//constexpr char const* REDIRECT_URI = "http://127.0.0.1/";
constexpr char const* CLIENT_ID = "http://localhost";

ATProtoTest::ATProtoTest(QObject* parent) : QObject(parent)
{
}

void ATProtoTest::oauth(const QString user, QString host)
{
    initOAuth(host);
    initHttpServer();

    QUrl clientUrl(CLIENT_ID);
    QUrlQuery query;
    query.addQueryItem("redirect_uri", REDIRECT_URI);
    query.addQueryItem("scope", OAUTH_SCOPE);
    clientUrl.setQuery(query);

    mClientId = clientUrl.toString();
    qDebug() << "client_id:" << mClientId;

    mOAuth->authorize(user, mClientId, REDIRECT_URI, OAUTH_SCOPE,
        [this](QString state, QString issuer, QUrl redirectUrl){
            qDebug() << "Authorize state:" << state << "issuer:" << issuer << "redirect:" << redirectUrl;
            mState = state;
            mIssuer = issuer;
            emit loginRedirect(redirectUrl);
        },
        [](int code, QString msg){
            qWarning() << "Authorize error:" << code << msg;
        });
}

void ATProtoTest::initOAuth(const QString& host)
{
    if (mOAuth)
        return;

    qDebug() << "Create dpop key";
    mDpopKey = JsonWebKey::generateDPoPKey();
    mOAuth = std::make_unique<OAuth>("https://" + host, &mDpopKey, this);
}

void ATProtoTest::initHttpServer()
{
    mHttpServer = std::make_unique<QHttpServer>(this);
    mHttpServer->route("/oauth/callback", this,
        [this](const QHttpServerRequest& request, QHttpServerResponder& responder){
            qDebug() << "oauth callback:" << request.url();
            requestToken(request.url());
            responder.write(QHttpServerResponder::StatusCode::NoContent);
        });

    mTcpServer = std::make_unique<QTcpServer>(this);

    if (!mTcpServer->listen(QHostAddress::LocalHost, LISTEN_PORT) || !mHttpServer->bind(mTcpServer.get()))
    {
        qWarning() << "Failed to list on port:" << LISTEN_PORT;
        return;
    }

    qDebug() << "Listening on port:" << mTcpServer->serverPort();
}

void ATProtoTest::requestToken(const QUrl& url)
{
    const QUrlQuery query(url.query());
    const QString state = query.queryItemValue("state", QUrl::FullyDecoded);
    const QString issuer = query.queryItemValue("iss", QUrl::FullyDecoded);
    const QString code = query.queryItemValue("code", QUrl::FullyDecoded);
    qDebug() << "state:" << state << "issuer:" << issuer << "code:" << code;

    if (state != mState)
    {
        qWarning() << "Unexpected state:" << state << "expected:" << mState;
        return;
    }

    if (issuer != mIssuer)
    {
        qWarning() << "Unexpected issuer:" << issuer << "expected:" << mIssuer;
        return;
    }

    mOAuth->initialTokenRequest(mClientId, REDIRECT_URI, code,
        [](QString did, QString scope, QString accessToken, QString refreshToken){
            qDebug() << "Token sucess did:" << did << "scope:" << scope << "access:" << accessToken << "refresh:" << refreshToken;
        },
        [](int code, QString error){
            qWarning() << "Token error:" << code << error;
        });
}

}
