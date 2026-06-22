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

    if (mAllowGroupInvites)
        json.insert("allowGroupInvites", AppBskyActor::allowIncomingTypeToString(*mAllowGroupInvites));

    return json;
}

Declaration::SharedPtr Declaration::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto declaration = std::make_shared<Declaration>();
    declaration->mAllowIncoming = AppBskyActor::stringToAllowIncomingType(xjson.getRequiredString("allowIncoming"));
    const auto allowGroup = xjson.getOptionalString("allowGroupInvites");

    if (allowGroup)
        declaration->mAllowGroupInvites = AppBskyActor::stringToAllowIncomingType(*allowGroup);

    declaration->mJson = json;
    return declaration;
}

MemberRole stringToMemberRole(const QString& str)
{
    static const std::unordered_map<QString, MemberRole> mapping = {
        { "owner", MemberRole::OWNER },
        { "standard", MemberRole::STANDARD }
    };

    return stringToEnum(str, mapping, MemberRole::UNKNOWN);
}

DirectConvoMember::SharedPtr DirectConvoMember::fromJson(const QJsonObject&)
{
    auto member = std::make_shared<DirectConvoMember>();
    return member;
}

GroupConvoMember::SharedPtr GroupConvoMember::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto member = std::make_shared<GroupConvoMember>();
    member->mAddedBy = xjson.getOptionalObject<ProfileViewBasic>("addedBy");
    member->mRawRole = xjson.getRequiredString("role");
    member->mRole = stringToMemberRole(member->mRawRole);
    return member;
}

PastGroupConvoMember::SharedPtr PastGroupConvoMember::fromJson(const QJsonObject&)
{
    auto member = std::make_shared<PastGroupConvoMember>();
    return member;
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
    profileViewBasic->mCreatedAt = root.getOptionalDateTime("createdAt");
    profileViewBasic->mChatDisabled = root.getOptionalBool("chatDisabled", false);
    profileViewBasic->mVerification = root.getOptionalObject<AppBskyActor::VerificationState>("verification");
    profileViewBasic->mKind = root.getOptionalVariant<
        DirectConvoMember,
        GroupConvoMember,
        PastGroupConvoMember,
        UnknownVariant>("kind");
    return profileViewBasic;
}

}
