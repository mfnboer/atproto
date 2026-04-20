// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "identity_resolver.h"
#include "plc_directory_client.h"
#include "presence.h"
#include "xrpc_network_thread.h"
#include "lexicon/com_atproto_server.h"
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace Xrpc {

class Client : public QObject, public ATProto::Presence
{
    Q_OBJECT

public:
    using Ptr = std::unique_ptr<Client>;
    using SetPdsSuccessCb = std::function<void()>;
    using SetPdsErrorCb = std::function<void(const QString& error)>;

    static constexpr int DEFAULT_TIMEOUT_MS = 10000;

    // Host can be set as first point of contact for a new account.
    // If handle to DID resolution via DNS fails, then createSession will be sent to host.
    // pdsDpopNonce is the last received nonce from previous session (OPTIONAL)
    explicit Client(const QString& host = {}, int networkTransferTimeoutMs = DEFAULT_TIMEOUT_MS, const QString& pdsDpopNonce = {});

    ~Client();

    ATProto::PlcDirectoryClient& getPlcDirectoryClient() { return mPlcDirectoryClient; }
    void setUserAgent(const QString& userAgent);
    const QString& getPDS() const { return mPDS; }
    void setPDS(const QString& pds, const QString& did);
    void enableOAuth(bool enable);
    bool isOAuthEnabled() const { return mOAuthEnabled; }
    void setDpopNonces(const QString& pdsDpopNonce, const QString& authDpopNonce);

    void setVideoHost(const QString& host);

    void setPDSFromSession(const ATProto::ComATProtoServer::Session& session);
    void setPDSFromDid(const QString& did, const SetPdsSuccessCb& successCb, const SetPdsErrorCb& errorCb);
    void setPDSFromHandle(const QString& handle, const SetPdsSuccessCb& successCb, const SetPdsErrorCb& errorCb);

    void post(const QString& service, const QJsonDocument& json, const NetworkThread::Params& rawHeaders,
              const NetworkThread::CallbackType& successCb, const NetworkThread::ErrorCb& errorCb,
              const QString& accessJwt = {}, bool isServiceAuthToken = false);
    void post(const QString& service, const NetworkThread::DataType& data, const QString& mimeType, const NetworkThread::Params& rawHeaders,
              const NetworkThread::SuccessJsonCb& successCb, const NetworkThread::ErrorCb& errorCb,
              const QString& accessJwt, bool isServiceAuthToken = false);
    void get(const QString& service, const NetworkThread::Params& params, const NetworkThread::Params& rawHeaders,
             const NetworkThread::CallbackType& successCb, const NetworkThread::ErrorCb& errorCb,
             const QString& accessJwt = {}, bool isServiceAuthToken = false, const QString& pds = {});

signals:
    // Internal use
    void postDataToNetwork(const QString& service, const NetworkThread::DataType& data, const QString& mimeType, const NetworkThread::Params& rawHeaders,
                           const NetworkThread::SuccessJsonCb& successCb, const NetworkThread::ErrorCb& errorCb,
                           const QString& accessJwt, bool isServiceAuthToken);
    void postJsonToNetwork(const QString& service, const QJsonDocument& json, const NetworkThread::Params& rawHeaders,
                           const NetworkThread::CallbackType& successCb, const NetworkThread::ErrorCb& errorCb,
                           const QString& accessJwt, bool isServiceAuthToken);
    void getToNetwork(const QString& service, const NetworkThread::Params& params, const NetworkThread::Params& rawHeaders,
                      const NetworkThread::CallbackType& successCb, const NetworkThread::ErrorCb& errorCb,
                      const QString& accessJwt, bool isServiceAuthToken, const QString& pds);
    void pdsChanged(const QString& pds);
    void oauthDisabled();
    void dpopNoncesChanged(const QString& pdsDpopNonce, const QString& authDpopNonce);
    void userAgentChanged(const QString& userAgent);
    void videoHostChanged(const QString& host);

    void oauthLogin(const QString& user, const QString& clientId, const QString& redirectUrl, const QStringList& scope,
                    const NetworkThread::OAuthLoginSuccessCb& successCb, const NetworkThread::OAuthErrorCb);
    void oauthRequestInitialToken(const QUrl& url,
                                  const NetworkThread::OAuthInitalTokenSuccessCb& successCb, const NetworkThread::OAuthErrorCb& errorCb);
    void oauthRefreshToken(const QString& refreshToken,
                           const NetworkThread::OAuthRefreshTokenSuccessCb& successCb, const NetworkThread::OAuthErrorCb& errorCb);
    void oauthResumeSession(const QString& clientId, const QString& refreshToken,
                            const NetworkThread::OAuthRefreshTokenSuccessCb& successCb, const NetworkThread::OAuthErrorCb& errorCb,
                            const QString& authDpopNonce = {});
    void oauthLogout(const QString& accessToken, const QString& refreshToken,
                     const NetworkThread::OAuthLogoutSuccessCb& successCb);

#if defined(Q_OS_ANDROID) && defined(USE_ANDROID_KEYSTORE)
    void oauthSetDpopKeyAlias(const QString& alias);
#else
    void oauthSaveDpopKey(const QString& path, const QString& passPhrase);
    void oauthLoadDpopKey(const QString& path, const QString& passPhrase);
#endif

    void pdsDpopNonceChanged(QString nonce);
    void authDpopNonceChanged(QString nonce);

private:
    template<typename CallbackType, typename ArgType>
    void doCallback(ArgType arg, CallbackType cb);

    QString mPDS;
    QString mDid; // PDS is set for this DID
    bool mOAuthEnabled = false;
    std::unique_ptr<QNetworkAccessManager> mNetwork;
    ATProto::PlcDirectoryClient mPlcDirectoryClient;
    ATProto::IdentityResolver mIdentityResolver;
    std::unique_ptr<NetworkThread> mNetworkThread;
};

}
