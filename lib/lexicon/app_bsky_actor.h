// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>

namespace ATProto::AppBskyActor {

struct ProfileViewDetailed
{
    QString mDid;
    QString mHandle;
    std::optional<QString> mDisplayName;
    std::optional<QString> mAvatar;
    std::optional<QString> mBanner;
    std::optional<QString> mDescription;
    std::optional<int> mFollowersCount;
    std::optional<int> mFollowsCount;
    std::optional<int> mPostsCount;
};

ProfileViewDetailed fromJson(const QJsonDocument& json);

}
