// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "test.h"
#include <QHostAddress>
#include <QTcpServer>

namespace ATProto {

static const QStringList TEST_SCOPE = { OAuth::SCPOPE_ATPROTO, OAuth::SCPOPE_BSKY_APP, OAuth::SCPOPE_BSKY_CHAT, OAuth::SCPOPE_TRANSITION_EMAIL };
constexpr char const* REDIRECT_URI = "http://127.0.0.1:1970/oauth/callback";
constexpr int LISTEN_PORT = 1970;
constexpr char const* CLIENT_ID = "http://localhost";

ATProtoTest::ATProtoTest(QObject* parent) : QObject(parent)
{
}

QString createClientId()
{
    const QString scope = TEST_SCOPE.join(' ');
    QUrl clientUrl(CLIENT_ID);
    QUrlQuery query;
    query.addQueryItem("redirect_uri", REDIRECT_URI);
    query.addQueryItem("scope", QUrl::toPercentEncoding(scope));
    clientUrl.setQuery(query);

    const QString clientId = clientUrl.toString();
    qDebug() << "client_id:" << clientId;
    return clientId;
}

void ATProtoTest::oauth(const QString user, QString host)
{
    auto xrpc = std::make_unique<Xrpc::Client>(host);
    mBsky = std::make_unique<ATProto::Client>(std::move(xrpc));
    initHttpServer();

    mBsky->oauthLogin(user, createClientId(), REDIRECT_URI, TEST_SCOPE,
        [this](QUrl redirectUrl){
            qDebug() << "Login success, redirect:" << redirectUrl;
            emit loginRedirect(redirectUrl);
        },
        [](QString error, QString msg){
            qWarning() << "Login error:" << error << "-" << msg;
        });
}

void ATProtoTest::initHttpServer()
{
    mHttpServer = std::make_unique<QHttpServer>(this);
    mHttpServer->route("/oauth/callback", this,
        [this](const QHttpServerRequest& request, QHttpServerResponder& responder){
            qDebug() << "oauth callback:" << request.url();
            loginContinue(request.url());
            responder.write(QHttpServerResponder::StatusCode::NoContent);
        });

    auto tcpServer = new QTcpServer(this);

    if (!mTcpServer->listen(QHostAddress::LocalHost, LISTEN_PORT) || !mHttpServer->bind(tcpServer))
    {
        qWarning() << "Failed to list on port:" << LISTEN_PORT;
        return;
    }

    qDebug() << "Listening on port:" << tcpServer->serverPort();
}

void ATProtoTest::loginContinue(const QUrl& url)
{
    const QUrlQuery query(url.query());
    const QString state = query.queryItemValue("state", QUrl::FullyDecoded);
    const QString issuer = query.queryItemValue("iss", QUrl::FullyDecoded);
    const QString code = query.queryItemValue("code", QUrl::FullyDecoded);
    const QString error = query.queryItemValue("error", QUrl::FullyDecoded);
    qDebug() << "state:" << state << "issuer:" << issuer << "code:" << code << "error:" << error;

    mBsky->oauthLoginContinue(url,
        [this](QString did, QString scope, QString accessToken, QString refreshToken){
            qDebug() << "Token sucess did:" << did << "scope:" << scope << "access:" << accessToken << "refresh:" << refreshToken;
            mUserDid = did;
            mAccessToken = accessToken;
            mRefreshToken = refreshToken;

            auto* s = mBsky->getSession();
            Q_ASSERT(s);
            qDebug() << "Session:" << s->mHandle << "did:" << s->mDid << "access:" << s->mAccessJwt << "refresh:" << s->mRefreshJwt << "email:" << s->mEmail.value_or("") << "didDoc:" << (s->mDidDoc ? "yes" : "no");

            refreshTokenRequest();
        },
        [](QString error, QString msg){
            qWarning() << "Token error:" << error << "-" << msg;
        });
}

void ATProtoTest::refreshTokenRequest()
{
    qDebug() << "Refresh token";
    mBsky->oauthRefreshToken(mRefreshToken,
        [this](QString accessToken, QString refreshToken){
            qDebug() << "Token refreshed access:" << accessToken << "refresh:" << refreshToken;
            mAccessToken = accessToken;
            mRefreshToken = refreshToken;
            logout();
        },
        [this](QString error, QString msg){
            qWarning() << "Token refresh error:" << error << "-" << msg;
            logout();
        });
}

void ATProtoTest::logout()
{
    qDebug() << "Logout";
    mBsky->oauthLogout(mAccessToken, mRefreshToken,
        [this]{
            qDebug() << "Logout succces";
        });
}

}
