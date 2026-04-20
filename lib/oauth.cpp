// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "oauth.h"
#include "network_utils.h"
#include "xjson.h"
#include <QCryptographicHash>
#include <QTimer>
#include <algorithm>

namespace ATProto {

static constexpr int MAX_DPOP_RESEND = 2;

static bool contains(const std::vector<QString>& list, const QString& value)
{
    return std::find(list.begin(), list.end(), value) != list.end();
}

ProtectedResourceMeta::Ptr ProtectedResourceMeta::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto meta = std::make_unique<ProtectedResourceMeta>();
    meta->mAuthorizationServers = xjson.getRequiredStringVector("authorization_servers");
    return meta;
}

AuthorizationServerMeta::Ptr AuthorizationServerMeta::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto meta = std::make_unique<AuthorizationServerMeta>();
    meta->mIssuer = xjson.getRequiredString("issuer");
    meta->mAuthorizationEndpoint = xjson.getRequiredString("authorization_endpoint");
    meta->mResponseTypesSupported = xjson.getRequiredStringVector("response_types_supported");
    meta->mGrantTypesSupported = xjson.getRequiredStringVector("grant_types_supported");
    meta->mCodeChallengeMethodsSupported = xjson.getRequiredStringVector("code_challenge_methods_supported");
    meta->mTokenEndpointAuthMedthodsSuppored = xjson.getRequiredStringVector("token_endpoint_auth_methods_supported");
    meta->mTokenEndpoint = xjson.getRequiredString("token_endpoint");
    meta->mScopesSupported = xjson.getRequiredStringVector("scopes_supported");
    meta->mAuthorizationResponeseIssParameterSupported = xjson.getRequiredBool("authorization_response_iss_parameter_supported");
    meta->mRequirePushedAuthorizationRequests = xjson.getRequiredBool("require_pushed_authorization_requests");
    meta->mPushedAuthorizationRequestEndpoint = xjson.getRequiredString("pushed_authorization_request_endpoint");
    meta->mDpopSigningAlgValuesSupported = xjson.getRequiredStringVector("dpop_signing_alg_values_supported");
    meta->mRequireRequestUriRegistration = xjson.getOptionalBool("require_request_uri_registration", true);
    meta->mClientIdMetadataDocumentSupported = xjson.getRequiredBool("client_id_metadata_document_supported");
    meta->mRevocationEndpoint = xjson.getOptionalString("revocation_endpoint");
    return meta;
}

OAuth::OAuth(const QString& pds,
             const QString& clientId,
             JsonWebKey* dpopPrivateJwk,
             QNetworkAccessManager* network,
             QObject* parent) :
    ATProto::NetworkClient<OAuthRequest, OAuthSuccessCb, OAuthErrorCb>(network, parent),
    mPds(pds),
    mClientId(clientId),
    mDpopPrivateJwk(dpopPrivateJwk)
{
    Q_ASSERT(mPds.startsWith("http"));
    qDebug() << "Created OAuth, PDS:" << pds << "clientId:" << mClientId;
}

void OAuth::setPds(const QString& pds)
{
    Q_ASSERT(pds.startsWith("http"));
    mPds = pds;
}

void OAuth::login(const QString& user, const QString& redirectUrl, const QStringList& scope,
                  const LoginSuccessCb& successCb, const ErrorCb& errorCb)
{
    const QString authScope = scope.join(' ');
    qDebug() << "Login:" << user << "redirectUrl:" << redirectUrl << "scope:" << authScope;

    getServerMetaData(
        [this, presence=getPresence(), user, redirectUrl, authScope, successCb, errorCb]{
            if (presence)
                authorizeContinuePAR(user, redirectUrl, authScope, successCb, errorCb);
        },
        errorCb);
}

std::optional<QNetworkRequest> OAuth::createNetworkRequest(const QString& url) const
{
    if (!NetworkUtils::isSafeUrl(url))
        return {};

    QNetworkRequest request(url);
    request.setMaximumRedirectsAllowed(0);
    setUserAgentHeader(request);

    // HACK:
    // HTTP/2 does not work between Qt6.10.2 and Eurosky.
    const QString host = request.url().host();
    if (host.endsWith("eurosky.social"))
        request.setAttribute(QNetworkRequest::Http2AllowedAttribute, false);

    return request;
}

void OAuth::authorizeContinuePAR(const QString& user, const QString& redirectUrl, const QString& scope,
                                 const LoginSuccessCb& successCb, const ErrorCb& errorCb)
{
    sendParAuthRequest(user, redirectUrl, scope,
        [this, presence=getPresence(), successCb](QString state, QString issuer, QString requestUri){
            if (!presence)
                return;

            qDebug() << "State:" << state << "issuer:" << issuer << "requestUri:" << requestUri;
            QUrlQuery query;
            query.addQueryItem("client_id", mClientId);
            query.addQueryItem("request_uri", requestUri);
            QUrl url(mAuthorizationServerMeta->mAuthorizationEndpoint);
            url.setQuery(query);
            successCb(state, issuer, url);
        },
        [errorCb](QString code, QString msg){
            errorCb(code, msg);
        });
}

void OAuth::getServerMetaData(const SuccessCb& successCb, const ErrorCb& errorCb)
{
    getProtectedResourceRequest(
        [this, presence=getPresence(), successCb, errorCb]{
            if (presence)
                getAuthorizationServerRequest(successCb, errorCb);
        },
        errorCb);
}

void OAuth::getProtectedResourceRequest(const SuccessCb& successCb, const ErrorCb& errorCb, int resendCount)
{
    const auto url = QString("%1/.well-known/oauth-protected-resource").arg(mPds);
    qDebug() << "Get protected resource:" << url;
    auto request = createNetworkRequest(url);

    if (!request)
    {
        errorCb(ERROR_INVALID_REQUEST, "Unsafe URL");
        return;
    }

    QNetworkReply* reply = mNetwork->get(*request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, successCb, errorCb, resendCount]{
        handleProtectedResourceResponse(reply, successCb, errorCb, resendCount);
    });
}

void OAuth::handleProtectedResourceResponse(QNetworkReply* reply, const SuccessCb& successCb, const ErrorCb& errorCb, int resendCount)
{
    qDebug() << "Reply:" << reply->url();

    if (reply->error() != QNetworkReply::NoError)
    {
        const QString& error = reply->errorString();
        qWarning() << "Failed to get:" << reply->url() << "code:" <<  reply->error() << "error:" << error;

        if (mustResend(reply->error()))
        {
            if (resendCount >= MAX_RESEND)
            {
                qWarning() << "Maximum resends reached:" << reply->url();
                errorCb(ERROR_TEMP_UNAVAILABLE, reply->errorString());
                return;
            }

            getProtectedResourceRequest(successCb, errorCb, resendCount + 1);
            return;
        }

        if (reply->error() == QNetworkReply::ContentNotFoundError)
        {
            // Assumed the PDS set is the authorization server
            mProtecedResourceMeta = std::make_unique<ProtectedResourceMeta>();
            mProtecedResourceMeta->mAuthorizationServers.push_back(mPds);
            successCb();
        }
        else
        {
            errorCb(ERROR_INVALID_REQUEST, reply->errorString());
        }
        return;
    }

    const auto status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);

    if (!status.isValid() || status.toInt() != 200)
    {
        qWarning() << "Did not receive 200 response:" << status.toInt();
        errorCb(ERROR_SERVER_ERROR, "Not a 200 response");
        return;
    }

    const auto data = reply->readAll();
    const QJsonDocument json(QJsonDocument::fromJson(data));

    if (!json.isObject())
    {
        qWarning() << "No JSON body";
        errorCb(ERROR_SERVER_ERROR, "No JSON body");
        return;
    }

    try {
        mProtecedResourceMeta = ProtectedResourceMeta::fromJson(json.object());
    } catch (InvalidJsonException& e) {
        qWarning() << e.msg();
        errorCb(ERROR_SERVER_ERROR, e.msg());
        return;
    }

    successCb();
}

void OAuth::getAuthorizationServerRequest(const SuccessCb& successCb, const ErrorCb& errorCb, int resendCount)
{
    const QString server = getAuthorizationServer();

    if (server.isEmpty())
    {
        errorCb(ERROR_INVALID_REQUEST, "No authorization server found");
        return;
    }

    const auto url = QString("%1/.well-known/oauth-authorization-server").arg(server);
    qDebug() << "Get protected resource:" << url;
    auto request = createNetworkRequest(url);

    if (!request)
    {
        errorCb(ERROR_INVALID_REQUEST, "Unsafe URL");
        return;
    }

    QNetworkReply* reply = mNetwork->get(*request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, successCb, errorCb, resendCount]{
        handleAuthorizatonServerResponse(reply, successCb, errorCb, resendCount);
    });
}

void OAuth::handleAuthorizatonServerResponse(QNetworkReply* reply, const SuccessCb& successCb, const ErrorCb& errorCb, int resendCount)
{
    qDebug() << "Reply:" << reply->url();

    if (reply->error() != QNetworkReply::NoError)
    {
        const QString& error = reply->errorString();
        qWarning() << "Failed to get:" << reply->url() << "code:" <<  reply->error() << "error:" << error;

        if (mustResend(reply->error()))
        {
            if (resendCount >= MAX_RESEND)
            {
                qWarning() << "Maximum resends reached:" << reply->url();
                errorCb(ERROR_TEMP_UNAVAILABLE, reply->errorString());
                return;
            }

            getAuthorizationServerRequest(successCb, errorCb, resendCount + 1);
            return;
        }

        errorCb(ERROR_INVALID_REQUEST, error);
        return;
    }

    const auto data = reply->readAll();
    const QJsonDocument json(QJsonDocument::fromJson(data));

    if (!json.isObject())
    {
        errorCb(ERROR_SERVER_ERROR, "No JSON body");
        return;
    }

    try {
        mAuthorizationServerMeta = AuthorizationServerMeta::fromJson(json.object());
    } catch (InvalidJsonException& e) {
        qWarning() << e.msg();
        errorCb(ERROR_SERVER_ERROR, e.msg());
        return;
    }

    if (!contains(mAuthorizationServerMeta->mResponseTypesSupported, "code"))
    {
        qWarning() << "code not supported:" << mAuthorizationServerMeta->mResponseTypesSupported;
        errorCb(ERROR_SERVER_ERROR, "response type 'code' not supported");
        return;
    }

    if (!contains(mAuthorizationServerMeta->mGrantTypesSupported, "authorization_code"))
    {
        qWarning() << "authorization_code not supported:" << mAuthorizationServerMeta->mGrantTypesSupported;
        errorCb(ERROR_SERVER_ERROR, "grant type 'authorization_code' not supported");
        return;
    }

    if (!contains(mAuthorizationServerMeta->mGrantTypesSupported, "refresh_token"))
    {
        qWarning() << "refresh_token not supported:" << mAuthorizationServerMeta->mGrantTypesSupported;
        errorCb(ERROR_SERVER_ERROR, "grant type 'refresh_token' not supported");
        return;
    }

    if (!contains(mAuthorizationServerMeta->mCodeChallengeMethodsSupported, "S256"))
    {
        qWarning() << "S256 not supported:" << mAuthorizationServerMeta->mCodeChallengeMethodsSupported;
        errorCb(ERROR_SERVER_ERROR, "code challenge method 'S256' not supported");
        return;
    }

    if (!contains(mAuthorizationServerMeta->mTokenEndpointAuthMedthodsSuppored, "none"))
    {
        qWarning() << "none not supported:" << mAuthorizationServerMeta->mTokenEndpointAuthMedthodsSuppored;
        errorCb(ERROR_SERVER_ERROR, "token endpoint auth method 'none' not supported");
        return;
    }

    if (!contains(mAuthorizationServerMeta->mScopesSupported, "atproto"))
    {
        qWarning() << "atproto not supported:" << mAuthorizationServerMeta->mScopesSupported;
        errorCb(ERROR_SERVER_ERROR, "scope 'atproto' not supported");
        return;
    }

    if (!mAuthorizationServerMeta->mAuthorizationResponeseIssParameterSupported)
    {
        qWarning() << "authorization_response_iss_parameter not supported";
        errorCb(ERROR_SERVER_ERROR, "authorization_response_iss_parameter not supported");
        return;
    }

    if (!mAuthorizationServerMeta->mRequirePushedAuthorizationRequests)
    {
        qWarning() << "pushed_authorization_requests required";
        errorCb(ERROR_SERVER_ERROR, "pushed_authorization_requests required");
        return;
    }

    if (!contains(mAuthorizationServerMeta->mDpopSigningAlgValuesSupported, "ES256"))
    {
        qWarning() << "ES256 not supported:" << mAuthorizationServerMeta->mDpopSigningAlgValuesSupported;
        errorCb(ERROR_SERVER_ERROR, "DPoP signing alg value 'ES256' not supported");
        return;
    }

    if (!mAuthorizationServerMeta->mRequireRequestUriRegistration)
    {
        qWarning() << "request_uri_registration required";
        errorCb(ERROR_SERVER_ERROR, "request_uri_registration required");
        return;
    }

    if (!mAuthorizationServerMeta->mClientIdMetadataDocumentSupported)
    {
        qWarning() << "client_id_metadata_document not supported";
        errorCb(ERROR_SERVER_ERROR, "client_id_metadata_document not supported");
        return;
    }

    if (!NetworkUtils::isSafeUrl(mAuthorizationServerMeta->mAuthorizationEndpoint))
    {
        qWarning() << "Unsafe authorization_endpoint";
        errorCb(ERROR_SERVER_ERROR, "Unsafe authorization_endpoint");
        return;
    }

    if (!NetworkUtils::isSafeUrl(mAuthorizationServerMeta->mTokenEndpoint))
    {
        qWarning() << "Unsafe token_endpoint";
        errorCb(ERROR_SERVER_ERROR, "Unsafe token_endpoint");
        return;
    }

    if (mAuthorizationServerMeta->mRevocationEndpoint && !NetworkUtils::isSafeUrl(*mAuthorizationServerMeta->mRevocationEndpoint))
    {
        qWarning() << "Unsafe revocation_endpoint";
        errorCb(ERROR_SERVER_ERROR, "Unsafe revocation_endpoint");
        return;
    }

    successCb();
}

QString OAuth::getAuthorizationServer() const
{
    if (!mProtecedResourceMeta || mProtecedResourceMeta->mAuthorizationServers.empty())
    {
        qWarning() << "No authorization server found";
        return {};
    }

    return mProtecedResourceMeta->mAuthorizationServers.front();
}

void OAuth::authServerPost(const QString& postUrl, const QUrlQuery& postData,
                    const AuthServerSuccessCb& successCb, const OAuthErrorCb& errorCb)
{
    qDebug() << "Post:" << postUrl << "data:" << postData.toString();
    Q_ASSERT(mDpopPrivateJwk);
    const QString dpopProof = mDpopPrivateJwk->buildAuthDPoPProof("POST", postUrl, mDpopNonce);

    auto networkRequest = createNetworkRequest(postUrl);

    if (!networkRequest)
    {
        errorCb(ERROR_INVALID_REQUEST, "Unsafe URL");
        return;
    }

    OAuthRequest request;
    request.mNetworkRequest = *networkRequest;
    request.mNetworkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.mNetworkRequest.setRawHeader("DPoP", dpopProof.toUtf8());
    request.mPostData = postData.toString().toUtf8();

    sendRequest(request, successCb, errorCb);
}

void OAuth::reportError(QNetworkReply* reply, const QByteArray& data, const OAuthErrorCb& errorCb)
{
    QJsonDocument json(QJsonDocument::fromJson(data));
    const XJsonObject xjson(json.object());
    const auto error = xjson.getOptionalString("error");

    if (error)
    {
        const auto errorDescription = xjson.getOptionalString("error_description", *error);
        errorCb(*error, errorDescription);
    }
    else
    {
        errorCb(ERROR_SERVER_ERROR, reply->errorString());
    }
}

void OAuth::replyFinished(const OAuthRequest& request, QNetworkReply* reply,
              const AuthServerSuccessCb& successCb, const OAuthErrorCb& errorCb,
              std::shared_ptr<bool> errorHandled)
{
    Q_ASSERT(reply);
    const auto errorCode = reply->error();
    const QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString().trimmed();
    qDebug() << "Reply:" << errorCode << "content:" << contentType;
    const bool hasDpopNonce = NetworkUtils::hasDpopNonce(reply);

    if (hasDpopNonce)
        setDpopNonce(NetworkUtils::getDpopNonce(reply));

    if (errorCode == QNetworkReply::NoError)
    {
        const auto data = reply->readAll();
        const QJsonDocument json(QJsonDocument::fromJson(data));
        successCb(json);
    }
    else if (!*errorHandled)
    {
        *errorHandled = true;
        const auto data = reply->readAll();

        if (mustResend(errorCode))
        {
            if (resendRequest(request, successCb, errorCb))
                return;
        }
        else if (NetworkUtils::isDpopNonceError(reply, data))
        {
            if (hasDpopNonce)
            {
                resendWithNewDpopNonce(request, successCb, errorCb);
            }
            else
            {
                qWarning() << "DPoP-Nonce missing";
                errorCb(ERROR_SERVER_ERROR, "DPoP-Nonce missing");
            }

            return;
        }

        reportError(reply, data, errorCb);
    }
    else
    {
        qDebug() << "Error already handled";
    }
}

void OAuth::networkError(const OAuthRequest& request, QNetworkReply* reply, QNetworkReply::NetworkError errorCode,
                  const AuthServerSuccessCb& successCb, const OAuthErrorCb& errorCb,
                  std::shared_ptr<bool> errorHandled)
{
    Q_ASSERT(reply);
    const auto errorMsg = reply->errorString();
    qWarning() << "Network error:" << errorCode << errorMsg;

    if (!*errorHandled)
    {
        *errorHandled = true;
        const auto data = reply->readAll();

        if (errorCode == QNetworkReply::OperationCanceledError)
            reply->disconnect();

        if (mustResend(errorCode))
        {
            qDebug() << "Try resend on error:" << errorCode << errorMsg;

            if (resendRequest(request, successCb, errorCb))
                return;
        }
        else if (NetworkUtils::isDpopNonceError(reply, data))
        {
            if (NetworkUtils::hasDpopNonce(reply))
            {
                setDpopNonce(NetworkUtils::getDpopNonce(reply));
                resendWithNewDpopNonce(request, successCb, errorCb);
            }
            else
            {
                qWarning() << "DPoP-Nonce missing";
                errorCb(ERROR_SERVER_ERROR, "DPoP-Nonce missing");
            }
            return;
        }

        reportError(reply, data, errorCb);
    }
    else
    {
        qDebug() << "Error already handled";
    }
}

void OAuth::sendParAuthRequest(const QString& user, const QString& redirectUrl, const QString& scope,
                               const ParSuccessCb& successCb, const ErrorCb& errorCb)
{
    Q_ASSERT(mAuthorizationServerMeta);
    qDebug() << "Send PAR:" << mAuthorizationServerMeta->mPushedAuthorizationRequestEndpoint << "user:" << user << "redirectUrl:" << redirectUrl << "scope:" << scope;
    const QString state = JsonWebKey::generateToken();
    mPkceVerifier = JsonWebKey::generateToken(48);
    const QString codeChallenge = createPkceCodeChallenge(mPkceVerifier);

    QUrlQuery parBody;
    parBody.addQueryItem("client_id", mClientId);
    parBody.addQueryItem("response_type", "code");
    parBody.addQueryItem("code_challenge", codeChallenge);
    parBody.addQueryItem("code_challenge_method", "S256");
    parBody.addQueryItem("state", state);
    parBody.addQueryItem("redirect_uri", redirectUrl);
    parBody.addQueryItem("scope", scope);

    if (!user.isEmpty())
        parBody.addQueryItem("login_hint", user);

    authServerPost(mAuthorizationServerMeta->mPushedAuthorizationRequestEndpoint, parBody,
        [presence=getPresence(), state, successCb, errorCb, this](QJsonDocument resp){
            if (!presence)
                return;

            qDebug() << "PAR success, state:" << state;

            const QJsonValue requestUri = resp["request_uri"];

            if (requestUri.isUndefined())
            {
                qWarning() << "request_uri missing";
                errorCb(ERROR_SERVER_ERROR, "no request_uri received from auth server");
                return;
            }

            qDebug() << "request_uri:" << requestUri.toString();
            successCb(state, mAuthorizationServerMeta->mIssuer, requestUri.toString());
        },
        [errorCb](QString errorCode, QString errorMsg){
            qDebug() << "PAR failed:" << errorCode << errorMsg;
            errorCb(errorCode, errorMsg);
        }
    );
}

void OAuth::initialTokenRequest(const QString& code, const QString& redirectUrl,
                                const InitialTokenSuccessCb& successCb, const ErrorCb& errorCb)
{
    Q_ASSERT(mAuthorizationServerMeta);
    qDebug() << "Inital token request:" << mAuthorizationServerMeta->mTokenEndpoint;
    QUrlQuery body;
    body.addQueryItem("client_id", mClientId);
    body.addQueryItem("redirect_uri", redirectUrl);
    body.addQueryItem("grant_type", "authorization_code");
    body.addQueryItem("code", code);
    body.addQueryItem("code_verifier", mPkceVerifier);

    authServerPost(mAuthorizationServerMeta->mTokenEndpoint, body,
        [presence=getPresence(), successCb, errorCb](QJsonDocument resp){
            if (!presence)
                return;

            qDebug() << "Token request success";

            try {
                XJsonObject xjson(resp.object());
                const QString sub = xjson.getRequiredString("sub");
                const QString scope = xjson.getRequiredString("scope");
                const QString accessToken = xjson.getRequiredString("access_token");
                const QString refreshToken = xjson.getRequiredString("refresh_token");
                successCb(sub, scope, accessToken, refreshToken);
            } catch (InvalidJsonException& e) {
                qWarning() << e.msg();
                errorCb(ERROR_SERVER_ERROR, e.msg());
            }
        },
        [errorCb](QString errorCode, QString errorMsg){
            qDebug() << "Token request failed:" << errorCode << errorMsg;
            errorCb(errorCode, errorMsg);
        }
    );
}

void OAuth::refreshTokenRequest(const QString& refreshToken,
                                const RefreshTokenSuccessCb& successCb, const ErrorCb& errorCb)
{
    Q_ASSERT(mAuthorizationServerMeta);
    qDebug() << "Refresh token request:" << mAuthorizationServerMeta->mTokenEndpoint << "token:" << refreshToken;
    QUrlQuery body;
    body.addQueryItem("client_id", mClientId);
    body.addQueryItem("grant_type", "refresh_token");
    body.addQueryItem("refresh_token", refreshToken);

    authServerPost(mAuthorizationServerMeta->mTokenEndpoint, body,
        [presence=getPresence(), successCb, errorCb](QJsonDocument resp){
            if (!presence)
                return;

            qDebug() << "Refresh token request success";

            try {
                XJsonObject xjson(resp.object());
                const QString accessToken = xjson.getRequiredString("access_token");
                const QString refreshToken = xjson.getRequiredString("refresh_token");
                successCb(accessToken, refreshToken);
            } catch (InvalidJsonException& e) {
                qWarning() << e.msg();
                errorCb(ERROR_SERVER_ERROR, e.msg());
            }
        },
        [errorCb](QString errorCode, QString errorMsg){
            qDebug() << "Refresh token request failed:" << errorCode << errorMsg;
            errorCb(errorCode, errorMsg);
        }
    );
}

void OAuth::resumeSession(const QString& refreshToken,
                          const RefreshTokenSuccessCb& successCb, const ErrorCb& errorCb,
                          const QString& dpopNonce)
{
    qDebug() << "Resume session";
    setDpopNonce(dpopNonce);

    getServerMetaData(
        [this, presence=getPresence(), refreshToken, successCb, errorCb]{
            if (presence)
                refreshTokenRequest(refreshToken, successCb, errorCb);
        },
        errorCb);
}

void OAuth::logout(const QString& accessToken, const QString& refreshToken,
                   const SuccessCb& successCb, const ErrorCb& errorCb)
{
    Q_ASSERT(mAuthorizationServerMeta);
    qDebug() << "Logout:" << mAuthorizationServerMeta->mRevocationEndpoint.value_or("<none>");

    if (!mAuthorizationServerMeta->mRevocationEndpoint)
    {
        qWarning() << "Revoking token not supprted";
        QTimer::singleShot(0, this, [errorCb]{ errorCb(ERROR_SERVER_ERROR, "Token revocation not supported"); });
        return;
    }

    revokeToken(accessToken, "access_token",
        [this, presence=getPresence(), refreshToken, successCb, errorCb]{
            if (presence)
                logoutContinue(refreshToken, successCb, errorCb);
        },
        [this, presence=getPresence(), refreshToken, successCb, errorCb](QString errorCode, QString errorMsg){
            if (presence)
                logoutContinue(refreshToken, successCb, errorCb, errorCode, errorMsg);
        });
}

void OAuth::logoutContinue(const QString& refreshToken,
                           const SuccessCb& successCb, const ErrorCb& errorCb,
                           std::optional<QString> errorCode, const QString& errorMsg)
{
    revokeToken(refreshToken, "refresh_token",
        [errorCode, errorMsg, successCb, errorCb]{
            if (errorCode)
                errorCb(*errorCode, errorMsg);
            else
                successCb();
        },
        [errorCb](QString errorCode, QString errorMsg){
            errorCb(errorCode, errorMsg);
        });
}

void OAuth::revokeToken(const QString& token, const QString& tokenType,
                        const SuccessCb& successCb, const ErrorCb& errorCb)
{
    Q_ASSERT(mAuthorizationServerMeta->mRevocationEndpoint);
    qDebug() << "Revoke token:" << tokenType;
    QUrlQuery body;
    body.addQueryItem("client_id", mClientId);
    body.addQueryItem("token", token);
    body.addQueryItem("token_type_hint", tokenType);

    authServerPost(*mAuthorizationServerMeta->mRevocationEndpoint, body,
        [presence=getPresence(), tokenType, successCb, errorCb](QJsonDocument){
            if (!presence)
                return;

            qDebug() << "Revoke token request success:" << tokenType;
            successCb();
        },
        [errorCb, tokenType](QString errorCode, QString errorMsg){
            qDebug() << "Revoke token request failed:" << tokenType << errorCode << errorMsg;
            errorCb(errorCode, errorMsg);
        }
    );
}

void OAuth::resendWithNewDpopNonce(const OAuthRequest& request,
                            const AuthServerSuccessCb& successCb, const OAuthErrorCb& errorCb)
{
    Q_ASSERT(!mDpopNonce.isEmpty());

    if (request.mDpopResendCount >= MAX_DPOP_RESEND)
    {
        qWarning() << "Max DPoP resends:" << request.mDpopResendCount;
        errorCb(ERROR_SERVER_ERROR, "Max DPoP resends");
        return;
    }

    const QString postUrl = request.mNetworkRequest.url().toString();
    qDebug() << "Resend:" << postUrl;
    OAuthRequest newRequest(request);
    ++newRequest.mDpopResendCount;
    qDebug() << "DPoP nonce:" << mDpopNonce;
    const QString dpopProof = mDpopPrivateJwk->buildAuthDPoPProof("POST", postUrl, mDpopNonce);
    qDebug() << "DPoP proof:" << dpopProof;
    newRequest.mNetworkRequest.setRawHeader("DPoP", dpopProof.toUtf8());
    sendRequest(newRequest, successCb, errorCb);
}

bool OAuth::resendRequest(OAuthRequest request, const AuthServerSuccessCb& successCb, const OAuthErrorCb& errorCb)
{
    const QString requestUrl = request.mNetworkRequest.url().toString();

    if (request.mResendCount >= MAX_RESEND)
    {
        qWarning() << "Maximum resends reached:" << requestUrl;
        return false;
    }

    ++request.mResendCount;
    qDebug() << "Resend:" << requestUrl << "count:" << request.mResendCount;

    if (request.mNetworkRequest.hasRawHeader("DPoP"))
    {
        // A new DPoP proof must be created, otherwise the resend will be seen as DPoP proof replay
        const QString dpopProof = mDpopPrivateJwk->buildAuthDPoPProof("POST", requestUrl, mDpopNonce);
    }

    sendRequest(request, successCb, errorCb);
    return true;
}

QString OAuth::createPkceCodeChallenge(const QString& verifier) const
{
    const auto hash = QCryptographicHash::hash(verifier.toUtf8(), QCryptographicHash::Sha256);
    return hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
}

void OAuth::setDpopNonce(const QString& nonce)
{
    if (nonce == mDpopNonce)
        return;

    mDpopNonce = nonce;
    emit dpopNonceChanged(mDpopNonce);
}

}
