// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "xrpc_client.h"

namespace ATProto {

struct UserProfile
{
    QString mDid;
    QString mHandle;
    QString mDisplayName;
    QString mAvatar;
    QString mBanner;
    QString mDescription;
    int mFollowersCount = 0;
    int mFollowsCount = 0;
    int mPostsCount = 0;
};

class Client
{
public:
    using createSessionSuccessCb = std::function<void()>;
    using getProfileSuccessCb = std::function<void(const UserProfile&)>;
    using ErrorCb = std::function<void(const QString& err)>;

    explicit Client(std::unique_ptr<Xrpc::Client>&& xrpc);

    void createSession(const QString& user, const QString& pwd,
                       const createSessionSuccessCb& successCb, const ErrorCb& errorCb);
    void getProfile(const QString& user, const getProfileSuccessCb& successCb, const ErrorCb& errorCb);

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
