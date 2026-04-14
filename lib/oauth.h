// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "json_web_key.h"
#include "network_client.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>
#include <QUrlQuery>

namespace ATProto {

struct ProtectedResourceMeta
{
    std::vector<QString> mAuthorizationServers;

    using Ptr = std::unique_ptr<ProtectedResourceMeta>;
    static Ptr fromJson(const QJsonObject& json);
};

struct AuthorizationServerMeta
{
    QString mIssuer;
    QString mAuthorizationEndpoint;
    std::vector<QString> mResponseTypesSupported;
    std::vector<QString> mGrantTypesSupported;
    std::vector<QString> mCodeChallengeMethodsSupported;
    std::vector<QString> mTokenEndpointAuthMedthodsSuppored;
    QString mTokenEndpoint;
    std::vector<QString> mScopesSupported;
    bool mAuthorizationResponeseIssParameterSupported = false;
    bool mRequirePushedAuthorizationRequests = false;
    QString mPushedAuthorizationRequestEndpoint;
    std::vector<QString> mDpopSigningAlgValuesSupported;
    bool mRequireRequestUriRegistration = true;
    bool mClientIdMetadataDocumentSupported = false;
    std::optional<QString> mRevocationEndpoint;

    using Ptr = std::unique_ptr<AuthorizationServerMeta>;
    static Ptr fromJson(const QJsonObject& json);
};

struct OAuthRequest
{
    QNetworkRequest mNetworkRequest;
    int mResendCount = 0;
    int mDpopResendCount = 0;
    bool mIsPost = true;
    QByteArray mPostData;
};

using OAuthSuccessCb = std::function<void(QJsonDocument resp)>;
using OAuthErrorCb = std::function<void(QString code, QString msg)>;

class OAuth : public NetworkClient<OAuthRequest, OAuthSuccessCb, OAuthErrorCb>
{
public:
    using AuthServerSuccessCb = OAuthSuccessCb;
    using LoginSuccessCb = std::function<void(QString state, QString issuer, QUrl redirectUrl)>;
    using ParSuccessCb = std::function<void(QString state, QString issuer, QString requestUri)>;
    using InitialTokenSuccessCb = std::function<void(QString did, QString scope, QString accessToken, QString refreshToken)>;
    using RefreshTokenSuccessCb = std::function<void(QString accessToken, QString refreshToken)>;
    using SuccessCb = std::function<void()>;
    using ErrorCb = std::function<void(QString code, QString msg)>;

    static constexpr char const* SCPOPE_ATPROTO = "atproto";
    static constexpr char const* SCPOPE_TRANSITION_GENERIC = "transition:generic";
    static constexpr char const* SCPOPE_TRANSITION_EMAIL = "transition:email";
    static constexpr char const* SCPOPE_TRANSITION_CHAT = "transition:chat.bsky";
    static constexpr char const* SCPOPE_BSKY_APP = "include:app.bsky.authFullApp?aud=did:web:api.bsky.app%23bsky_appview";
    static constexpr char const* SCPOPE_BSKY_CHAT = "include:chat.bsky.authFullChatClient?aud=did:web:api.bsky.chat%23bsky_chat";

    static constexpr char const* ERROR_INVALID_REQUEST = "invalid_request";
    static constexpr char const* ERROR_INVALID_CLIENT = "invalid_client";
    static constexpr char const* ERROR_INVALID_GRANT = "invalid_grant";
    static constexpr char const* ERROR_UNAUTHORIZED_CLIENT = "unauthorized_client";
    static constexpr char const* ERROR_UNSUPPORTED_GRANT_TYPE = "unsupported_grant_type";
    static constexpr char const* ERROR_INVALID_SCOPE = "invalid_scope";
    static constexpr char const* ERROR_ACCESS_DENIED = "access_denied";
    static constexpr char const* ERROR_UNSUPPORTED_RESPONSE_TYPE = "unsupported_repsonse_type";
    static constexpr char const* ERROR_SERVER_ERROR = "server_error";
    static constexpr char const* ERROR_TEMP_UNAVAILABLE = "temporarily_unavailable";

    /**
     * @brief OAuth
     * @param handle handle or DID. May be empty.
     * @param pds This can be an entry point is handle is empty, e.g. https://bsky.social
     * @param dpopPrivateJwk
     * @param parent
     */
    OAuth(const QString& user, const QString& pds,
          const QString& clientId, const QString& redirectUrl,
          JsonWebKey* dpopPrivateJwk,
          QNetworkAccessManager* network,
          QObject* parent = nullptr);

    void setPds(const QString& pds);

    /**
     * @brief login
     * @param scope
     * @param successCb
     * @param errorCb
     *
     * Gets meta data for the authorizaion server.
     */
    void login(const QStringList& scope,
               const LoginSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief initialTokenRequest
     * @param code code from login
     * @param successCb
     * @param errorCb
     */
    void initialTokenRequest(const QString& code,
                             const InitialTokenSuccessCb& successCb, const ErrorCb& errorCb);

    void refreshTokenRequest(const QString& refreshToken,
                             const RefreshTokenSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief resumeSession get meta data and refresh
     * @param refreshToken
     * @param successCb
     * @param errorCb
     *
     * Gets meta data for the authorizaion server.
     *
     */
    void resumeSession(const QString& refreshToken,
                       const RefreshTokenSuccessCb& successCb, const ErrorCb& errorCb);

    void logout(const QString& accessToken, const QString& refreshToken,
                const SuccessCb& successCb, const ErrorCb& errorCb);

    const QString& getDpopNonce() const { return mDpopNonce; }

private:
    std::optional<QNetworkRequest> createNetworkRequest(const QString& url) const;

    void authorizeContinue(const QString& scope,
                           const LoginSuccessCb& successCb, const ErrorCb& errorCb);
    void authorizeContinuePAR(const QString& scope,
                              const LoginSuccessCb& successCb, const ErrorCb& errorCb);

    void sendParAuthRequest(const QString& scope,
                            const ParSuccessCb& successCb, const ErrorCb& errorCb);

    void getServerMetaData(const SuccessCb& successCb, const ErrorCb& errorCb);
    void getProtectedResourceRequest(const SuccessCb& successCb, const ErrorCb& errorCb);
    void handleProtectedResourceResponse(QNetworkReply* reply, const SuccessCb& successCb, const ErrorCb& errorCb);
    void getAuthorizationServerRequest(const SuccessCb& successCb, const ErrorCb& errorCb);
    void handleAuthorizatonServerResponse(QNetworkReply* reply, const SuccessCb& successCb, const ErrorCb& errorCb);

    QString getAuthorizationServer() const;

    void authServerPost(const QString& postUrl, const QUrlQuery& postData,
                        const AuthServerSuccessCb& successCb, const OAuthErrorCb& errorCb);

    void reportError(QNetworkReply* reply, const QByteArray& data, const OAuthErrorCb& errorCb);

    virtual void replyFinished(const OAuthRequest& request, QNetworkReply* reply,
                               const AuthServerSuccessCb& successCb, const OAuthErrorCb& errorCb,
                               std::shared_ptr<bool> errorHandled) override;

    virtual void networkError(const OAuthRequest& request, QNetworkReply* reply, QNetworkReply::NetworkError errorCode,
                              const AuthServerSuccessCb& successCb, const OAuthErrorCb& errorCb,
                              std::shared_ptr<bool> errorHandled) override;

    void logoutContinue(const QString& refreshToken,
                        const SuccessCb& successCb, const ErrorCb& errorCb,
                        std::optional<QString> errorCode = {}, const QString& errorMsg = {});
    void revokeToken(const QString& token, const QString& tokenType,
                     const SuccessCb& successCb, const ErrorCb& errorCb);
    void resendWithNewDpopNonce(const OAuthRequest& request,
                                const AuthServerSuccessCb& successCb, const OAuthErrorCb& errorCb);

    QString createPkceCodeChallenge(const QString& verifier) const;

    QString mLoginHint;
    QString mPds;
    ProtectedResourceMeta::Ptr mProtecedResourceMeta;
    AuthorizationServerMeta::Ptr mAuthorizationServerMeta;
    QString mClientId;
    QString mClientRedirectUrl;
    JsonWebKey* mDpopPrivateJwk = nullptr;
    QString mDpopNonce;
    QString mPkceVerifier;
};

}
