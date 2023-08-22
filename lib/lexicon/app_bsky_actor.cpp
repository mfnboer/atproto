// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_actor.h"
#include "../xjson.h"

namespace ATProto::AppBskyActor {

ProfileViewBasic::Ptr ProfileViewBasic::fromJson(const QJsonObject& json)
{
    XJsonObject root(json);
    auto profileViewBasic = std::make_unique<ProfileViewBasic>();
    profileViewBasic->mDid = root.getRequiredString("did");
    profileViewBasic->mHandle = root.getRequiredString("handle");
    profileViewBasic->mDisplayName = root.getOptionalString("displayName");
    profileViewBasic->mAvatar = root.getOptionalString("avatar");
    return profileViewBasic;
}

ProfileViewDetailed::Ptr ProfileViewDetailed::fromJson(const QJsonDocument& json)
{
    const auto jsonObj = json.object();
    XJsonObject root(jsonObj);
    auto profile = std::make_unique<ProfileViewDetailed>();
    profile->mDid = root.getRequiredString("did");
    profile->mHandle = root.getRequiredString("handle");
    profile->mDisplayName = root.getOptionalString("displayName");
    profile->mAvatar = root.getOptionalString("avatar");
    profile->mBanner = root.getOptionalString("banner");
    profile->mDescription = root.getOptionalString("description");
    profile->mFollowersCount = root.getOptionalInt("followersCount");
    profile->mFollowsCount = root.getOptionalInt("followsCount");
    profile->mPostsCount = root.getOptionalInt("postsCount");
    profile->mIndexedAt = root.getOptionalDateTime("indexedAt");
    return profile;
}

}
