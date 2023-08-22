// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>

namespace ATProto::AppBskyActor {

// app.bsky.actor.defs#profileViewBasic
struct ProfileViewBasic
{
    QString mDid;
    QString mHandle;
    std::optional<QString> mDisplayName; // max 64 graphemes, 640 bytes
    std::optional<QString> mAvatar; // URL
    // TODO viewer
    // TODO labels

    using Ptr = std::unique_ptr<ProfileViewBasic>;
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
    std::optional<int> mFollowersCount;
    std::optional<int> mFollowsCount;
    std::optional<int> mPostsCount;
    std::optional<QDateTime> mIndexedAt;
    // TODO viewer
    // TODO labels

    using Ptr = std::unique_ptr<ProfileViewDetailed>;
    static Ptr fromJson(const QJsonDocument& json);
};

}
