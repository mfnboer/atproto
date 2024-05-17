// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include <QJsonDocument>

namespace ATProto::ChatBskyActor {

enum class AllowIncomingType
{
    ALL,
    NONE,
    FOLLOWING
};
AllowIncomingType stringToAllowIncomingType(const QString& str);
QString allowIncomingTypeToString(AllowIncomingType allowIncoming);

// chat.bsky.actor.declaration
struct Declaration
{
    AllowIncomingType mAllowIncoming;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Declaration>;
    static Ptr fromJson(const QJsonObject& json);
};

// chat.bsky.actor.defs#profileViewBasic
struct ProfileViewBasic
{
    QString mDid;
    QString mHandle;
    std::optional<QString> mDisplayName; // max 64 graphemes, 640 bytes
    std::optional<QString> mAvatar; // URL
    AppBskyActor::ProfileAssociated::Ptr mAssociated; // optional
    AppBskyActor::ViewerState::Ptr mViewer; // optional
    std::vector<ComATProtoLabel::Label::Ptr> mLabels;
    bool mChatDisabled = false;

    using Ptr = std::unique_ptr<ProfileViewBasic>;
    static Ptr fromJson(const QJsonObject& json);
};

using ProfileViewBasicList = std::vector<ProfileViewBasic::Ptr>;

}
