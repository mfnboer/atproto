// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "com_atproto_label.h"
#include <QJsonDocument>

namespace ATProto::AppBskyActor {

// app.bsky.actor.defs#viewerState
struct ViewerState
{
    bool mMuted = false;
    bool mBlockedBy = false;
    // NOT IMPLEMENTED following
    // NOT IMPLEMENTED followedBy

    using Ptr = std::unique_ptr<ViewerState>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#profileViewBasic
struct ProfileViewBasic
{
    QString mDid;
    QString mHandle;
    std::optional<QString> mDisplayName; // max 64 graphemes, 640 bytes
    std::optional<QString> mAvatar; // URL
    ViewerState::Ptr mViewer; // optional
    std::vector<ComATProtoLabel::Label::Ptr> mLabels;

    using Ptr = std::unique_ptr<ProfileViewBasic>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#profileView
struct ProfileView
{
    QString mDid;
    QString mHandle;
    std::optional<QString> mDisplayName; // max 64 graphemes, 640 bytes
    std::optional<QString> mAvatar; // URL
    std::optional<QString> mDescription; // max 256 graphemes, 2560 bytes
    std::optional<QDateTime> mIndexedAt;
    ViewerState::Ptr mViewer; // optional
    std::vector<ComATProtoLabel::Label::Ptr> mLabels;

    using Ptr = std::unique_ptr<ProfileView>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#profileViewDetailed
struct ProfileViewDetailed
{
    QString mDid;
    QString mHandle;
    std::optional<QString> mDisplayName; // max 64 graphemes, 640 bytes
    std::optional<QString> mAvatar; // URL
    std::optional<QString> mBanner; // URL
    std::optional<QString> mDescription; // max 256 graphemes, 2560 bytes
    int mFollowersCount = 0;
    int mFollowsCount = 0;
    int mPostsCount = 0;
    std::optional<QDateTime> mIndexedAt;
    ViewerState::Ptr mViewer; // optional
    std::vector<ComATProtoLabel::Label::Ptr> mLabels;

    using Ptr = std::unique_ptr<ProfileViewDetailed>;
    static Ptr fromJson(const QJsonDocument& json);
};

}
