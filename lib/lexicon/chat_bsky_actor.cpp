// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "chat_bsky_actor.h"
#include "../xjson.h"

namespace ATProto::ChatBskyActor {

QJsonObject Declaration::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", TYPE);
    json.insert("allowIncoming", AppBskyActor::allowIncomingTypeToString(mAllowIncoming));
    return json;
}

Declaration::SharedPtr Declaration::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto declaration = std::make_shared<Declaration>();
    declaration->mAllowIncoming = AppBskyActor::stringToAllowIncomingType(xjson.getRequiredString("allowIncoming"));
    declaration->mJson = json;
    return declaration;
}

ProfileViewBasic::SharedPtr ProfileViewBasic::fromJson(const QJsonObject& json)
{
    XJsonObject root(json);
    auto profileViewBasic = std::make_shared<ProfileViewBasic>();
    profileViewBasic->mDid = root.getRequiredString("did");
    profileViewBasic->mHandle = root.getRequiredString("handle");
    profileViewBasic->mDisplayName = root.getOptionalString("displayName");
    profileViewBasic->mAvatar = root.getOptionalString("avatar");
    profileViewBasic->mAssociated = root.getOptionalObject<AppBskyActor::ProfileAssociated>("associated");
    profileViewBasic->mViewer = root.getOptionalObject<AppBskyActor::ViewerState>("viewer");
    ComATProtoLabel::getLabels(profileViewBasic->mLabels, json);
    profileViewBasic->mChatDisabled = root.getOptionalBool("chatDisabled", false);
    return profileViewBasic;
}

}
