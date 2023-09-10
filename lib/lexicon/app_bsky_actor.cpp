// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_actor.h"
#include "../xjson.h"

namespace ATProto::AppBskyActor {

ViewerState::Ptr ViewerState::fromJson(const QJsonObject& json)
{
    auto viewerState = std::make_unique<ViewerState>();
    XJsonObject xjson(json);
    viewerState->mMuted = xjson.getOptionalBool("muted", false);
    viewerState->mBlockedBy = xjson.getOptionalBool("blockedBy", false);
    return viewerState;
}

ProfileViewBasic::Ptr ProfileViewBasic::fromJson(const QJsonObject& json)
{
    XJsonObject root(json);
    auto profileViewBasic = std::make_unique<ProfileViewBasic>();
    profileViewBasic->mDid = root.getRequiredString("did");
    profileViewBasic->mHandle = root.getRequiredString("handle");
    profileViewBasic->mDisplayName = root.getOptionalString("displayName");
    profileViewBasic->mAvatar = root.getOptionalString("avatar");

    const auto viewerJson = root.getOptionalObject("viewer");
    if (viewerJson)
        profileViewBasic->mViewer = ViewerState::fromJson(*viewerJson);

    ComATProtoLabel::getLabels(profileViewBasic->mLabels, json);
    return profileViewBasic;
}

ProfileView::Ptr ProfileView::fromJson(const QJsonObject& json)
{
    XJsonObject root(json);
    auto profile = std::make_unique<ProfileView>();
    profile->mDid = root.getRequiredString("did");
    profile->mHandle = root.getRequiredString("handle");
    profile->mDisplayName = root.getOptionalString("displayName");
    profile->mAvatar = root.getOptionalString("avatar");
    profile->mDescription = root.getOptionalString("description");
    profile->mIndexedAt = root.getOptionalDateTime("indexedAt");

    const auto viewerJson = root.getOptionalObject("viewer");
    if (viewerJson)
        profile->mViewer = ViewerState::fromJson(*viewerJson);

    ComATProtoLabel::getLabels(profile->mLabels, json);
    return profile;
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
    profile->mFollowersCount = root.getOptionalInt("followersCount", 0);
    profile->mFollowsCount = root.getOptionalInt("followsCount", 0);
    profile->mPostsCount = root.getOptionalInt("postsCount", 0);
    profile->mIndexedAt = root.getOptionalDateTime("indexedAt");

    const auto viewerJson = root.getOptionalObject("viewer");
    if (viewerJson)
        profile->mViewer = ViewerState::fromJson(*viewerJson);

    ComATProtoLabel::getLabels(profile->mLabels, jsonObj);
    return profile;
}

}
