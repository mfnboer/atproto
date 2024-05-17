// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "chat_bsky_actor.h"
#include "../xjson.h"
#include <unordered_map>

namespace ATProto::ChatBskyActor {

AllowIncomingType stringToAllowIncomingType(const QString& str)
{
    static const std::unordered_map<QString, AllowIncomingType> mapping = {
        { "all", AllowIncomingType::ALL },
        { "none", AllowIncomingType::NONE },
        { "following", AllowIncomingType::FOLLOWING }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    return AllowIncomingType::NONE;
}

QString allowIncomingTypeToString(AllowIncomingType allowIncoming)
{
    static const std::unordered_map<AllowIncomingType, QString> mapping = {
        { AllowIncomingType::ALL, "all" },
        { AllowIncomingType::NONE, "none" },
        { AllowIncomingType::FOLLOWING, "following" }
    };

    const auto it = mapping.find(allowIncoming);
    Q_ASSERT(it != mapping.end());

    if (it == mapping.end())
    {
        qWarning() << "Unknown allow incoming type:" << int(allowIncoming);
        return "none";
    }

    return it->second;
}

QJsonObject Declaration::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", "chat.bsky.actor.declaration");
    json.insert("allowIncoming", allowIncomingTypeToString(mAllowIncoming));
    return json;
}

Declaration::Ptr Declaration::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto declaration = std::make_unique<Declaration>();
    declaration->mAllowIncoming = stringToAllowIncomingType(xjson.getRequiredString("allowIncoming"));
    declaration->mJson = json;
    return declaration;
}

ProfileViewBasic::Ptr ProfileViewBasic::fromJson(const QJsonObject& json)
{
    XJsonObject root(json);
    auto profileViewBasic = std::make_unique<ProfileViewBasic>();
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
