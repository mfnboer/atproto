// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include <QJsonDocument>

namespace ATProto::AppBskyGraph {

// app.bsky.graph.getFollow/output
struct GetFollowsOutput
{
    AppBskyActor::ProfileView::Ptr mSubject;
    std::vector<AppBskyActor::ProfileView::Ptr> mFollows;
    std::optional<QString> mCursor;

    using Ptr = std::unique_ptr<GetFollowsOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.graph.follow
struct Follow
{
    QString mSubject; // did
    QDateTime mCreatedAt;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Follow>;
    static Ptr fromJson(const QJsonObject& json);
};

}
