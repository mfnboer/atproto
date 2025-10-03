// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include <QJsonDocument>

namespace ATProto::ChatBskyActor {

// chat.bsky.actor.declaration
struct Declaration
{
    AppBskyActor::AllowIncomingType mAllowIncoming;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Declaration>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.actor.declaration";
};

// chat.bsky.actor.defs#profileViewBasic
struct ProfileViewBasic
{
    QString mDid;
    QString mHandle;
    std::optional<QString> mDisplayName; // max 64 graphemes, 640 bytes
    std::optional<QString> mAvatar; // URL
    AppBskyActor::ProfileAssociated::SharedPtr mAssociated; // optional
    AppBskyActor::ViewerState::SharedPtr mViewer; // optional
    ComATProtoLabel::Label::List mLabels;
    bool mChatDisabled = false;

    using SharedPtr = std::shared_ptr<ProfileViewBasic>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}
