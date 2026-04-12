// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "test.h"
#include <QHostAddress>

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
        [](QString msg){
            qWarning() << "Login error:" << msg;
        });
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

    mBsky->oauthRequestInitialToken(url,
        [this](QString did, QString scope, QString accessToken, QString refreshToken){
            qDebug() << "Token sucess did:" << did << "scope:" << scope << "access:" << accessToken << "refresh:" << refreshToken;
            mUserDid = did;
            mAccessToken = accessToken;
            mRefreshToken = refreshToken;
            refreshTokenRequest();
        },
        [](QString error){
            qWarning() << "Token error:" << error;
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
            getSession();
        },
        [](QString error){
            qWarning() << "Token refresh error:" << error;
        });
}

void ATProtoTest::getSession()
{
    qDebug() << "Get session";
    ComATProtoServer::Session session;
    session.mDid = mUserDid;
    session.mAccessJwt = mAccessToken;
    session.mRefreshJwt = mRefreshToken;

    mBsky->resumeSession(session,
        [this]{
            auto* s = mBsky->getSession();
            Q_ASSERT(s);
            qDebug() << "Session:" << s->mHandle << "did:" << s->mDid << "access:" << s->mAccessJwt << "refresh:" << s->mRefreshJwt << "email:" << s->mEmail.value_or("") << "didDoc:" << (s->mDidDoc ? "yes" : "no");
            logout();
        },
        [this](const QString& error, const QString& msg){
            qWarning() << "Get session failed:" << error << " - " << msg;
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
