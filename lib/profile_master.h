// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "client.h"
#include "presence.h"
#include "lexicon/app_bsky_actor.h"

namespace ATProto {

class ProfileMaster : public Presence
{
public:
    using ProfileCb = std::function<void(AppBskyActor::Profile::Ptr)>;
    using SuccessCb = Client::SuccessCb;
    using ErrorCb = Client::ErrorCb;

    static bool hasLabel(const ATProto::AppBskyActor::ProfileView& profileView, const QString& label);
    static bool getLoggedOutVisibility(const ATProto::AppBskyActor::ProfileView& profileView);

    explicit ProfileMaster(Client& client);

    void getProfile(const QString& did, const ProfileCb& successCb, const ErrorCb& errorCb);
    void updateProfile(const QString& did, const AppBskyActor::Profile& profile,
                       const SuccessCb& successCb, const ErrorCb& errorCb);
    void updateProfile(const QString& did, const QString& name, const QString& description,
                       Blob::Ptr avatar, bool updateAvatar, Blob::Ptr banner, bool updatebanner,
                       const SuccessCb& successCb, const ErrorCb& errorCb);

    void addSelfLabel(const QString& did, const QString& label,
                      const SuccessCb& successCb, const ErrorCb& errorCb);
    void removeSelfLabel(const QString& did, const QString& label,
                         const SuccessCb& successCb, const ErrorCb& errorCb);

    void setLoggedOutVisibility(const QString& did, bool enable,
                                const SuccessCb& successCb, const ErrorCb& errorCb);

private:
    bool addLabel(AppBskyActor::Profile& profile, const QString& label) const;
    bool removeLabel(AppBskyActor::Profile& profile, const QString& label) const;

    Client& mClient;
    std::unordered_map<QString, Blob::Ptr> mDidAvatarBlobMap;
    std::unordered_map<QString, Blob::Ptr> mDidBannerBlobMap;
};

}
