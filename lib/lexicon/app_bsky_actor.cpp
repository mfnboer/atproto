// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_actor.h"
#include "../xjson.h"

namespace ATProto::AppBskyActor {

ProfileViewDetailed fromJson(const QJsonDocument& json)
{
    XJsonObject root(json.object());
    ProfileViewDetailed profile;
    profile.mDid = root.getRequiredString("did");
    profile.mHandle = root.getRequiredString("handle");
    profile.mDisplayName = root.getOptionalString("displayName");
    profile.mAvatar = root.getOptionalString("avatar");
    profile.mBanner = root.getOptionalString("banner");
    profile.mDescription = root.getOptionalString("description");
    profile.mFollowersCount = root.getOptionalInt("followersCount");
    profile.mFollowsCount = root.getOptionalInt("followsCount");
    profile.mPostsCount = root.getOptionalInt("postsCount");
    return profile;
}

}
