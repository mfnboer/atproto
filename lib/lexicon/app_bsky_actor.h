// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>
#include <QString>

namespace ATProto::AppBskyActor {

struct ProfileViewDetailed
{
    QString mDid;
    QString mHandle;
    QString mDisplayName;
    QString mAvatar;
    QString mBanner;
    QString mDescription;
    int mFollowersCount = 0;
    int mFollowsCount = 0;
    int mPostsCount = 0;
};

ProfileViewDetailed fromJson(const QJsonDocument& json);

}
