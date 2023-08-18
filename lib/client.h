// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "xrpc_client.h"
#include "lexicon/app_bsky_actor.h"

namespace ATProto {

class Client
{
public:
    using createSessionSuccessCb = std::function<void()>;
    using getProfileSuccessCb = std::function<void(const AppBskyActor::ProfileViewDetailed&)>;
    using ErrorCb = std::function<void(const QString& err)>;

    explicit Client(std::unique_ptr<Xrpc::Client>&& xrpc);

    // com.atproto.server
    /**
     * @brief createSession
     * @param user user handle or did
     * @param pwd password
     * @param successCb
     * @param errorCb
     */
    void createSession(const QString& user, const QString& pwd,
                       const createSessionSuccessCb& successCb, const ErrorCb& errorCb);

    // com.bsky.actor
    /**
     * @brief getProfile
     * @param user user handle or did
     * @param successCb
     * @param errorCb
     */
    void getProfile(const QString& user, const getProfileSuccessCb& successCb, const ErrorCb& errorCb);

    // app.bsky.feed
    /**
     * @brief getAuthorFeed
     * @param user user handle or did
     * @param limit min=1 max=100 default=50
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getAuthorFeed(const QString& user, std::optional<int> limit, const std::optional<QString>& cursor,
                       const createSessionSuccessCb& successCb, const ErrorCb& errorCb);

private:
    void sessionCreated(const QJsonDocument& json, const createSessionSuccessCb& successCb, const ErrorCb& errorCb);
    void getProfileSuccess(const QJsonDocument& json, const getProfileSuccessCb& successCb, const ErrorCb& errorCb);
    void requestFailed(const QString& err, const QJsonDocument& json, const ErrorCb& errorCb);

    std::unique_ptr<Xrpc::Client> mXrpc;
    bool mSessionCreated = false;
    QString mUserHandle;
    QString mUserDid;
    QString mAccessJwt;
    QString mRefreshJwt;
};

}
