// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "client.h"
#include "presence.h"
#include "repo_master.h"
#include "lexicon/app_bsky_actor.h"

namespace ATProto {

class ProfileMaster : public Presence
{
public:
    using ProfileCb = std::function<void(AppBskyActor::Profile::SharedPtr)>;
    using RecordSuccessCb = std::function<void(const QString& uri, const QString& cid)>;
    using StatusCb = std::function<void(AppBskyActor::Status::SharedPtr)>;
    using SuccessCb = Client::SuccessCb;
    using ErrorCb = Client::ErrorCb;

    static bool hasLabel(const ATProto::AppBskyActor::ProfileViewDetailed& profileView, const QString& label);
    static bool getLoggedOutVisibility(const ATProto::AppBskyActor::ProfileViewDetailed& profileView);

    explicit ProfileMaster(Client& client);

    void getProfile(const QString& did, const ProfileCb& successCb, const ErrorCb& errorCb);
    void updateProfile(const QString& did, const AppBskyActor::Profile& profile,
                       const SuccessCb& successCb, const ErrorCb& errorCb);
    void updateProfile(const QString& did, const QString& name, const QString& description,
                       Blob::SharedPtr avatar, bool updateAvatar, Blob::SharedPtr banner, bool updatebanner,
                       const QString& pronouns, const QString& website,
                       const SuccessCb& successCb, const ErrorCb& errorCb);

    void addSelfLabel(const QString& did, const QString& label,
                      const SuccessCb& successCb, const ErrorCb& errorCb);
    void removeSelfLabel(const QString& did, const QString& label,
                         const SuccessCb& successCb, const ErrorCb& errorCb);

    void setLoggedOutVisibility(const QString& did, bool enable,
                                const SuccessCb& successCb, const ErrorCb& errorCb);

    void setPinnedPost(const QString& did, const QString& uri, const QString& cid,
                       const SuccessCb& successCb, const ErrorCb& errorCb);
    void clearPinnedPost(const QString& did, const SuccessCb& successCb, const ErrorCb& errorCb);

    void getStatus(const QString& did, const StatusCb& successCb, const ErrorCb& errorCb);
    void updateStatus(const QString& did, const AppBskyActor::Status& status,
                      const SuccessCb& successCb, const ErrorCb& errorCb);
    void deleteStatus(const QString& did, const SuccessCb& successCb, const ErrorCb& errorCb);

private:
    bool addLabel(AppBskyActor::Profile& profile, const QString& label) const;
    bool removeLabel(AppBskyActor::Profile& profile, const QString& label) const;
    bool setPinnedPost(AppBskyActor::Profile& profile, const QString& uri, const QString& cid) const;
    bool clearPinnedPost(AppBskyActor::Profile& profile) const;

    Client& mClient;
    RepoMaster mRepoMaster;
    std::unordered_map<QString, Blob::SharedPtr> mDidAvatarBlobMap;
    std::unordered_map<QString, Blob::SharedPtr> mDidBannerBlobMap;
};

}
