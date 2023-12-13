// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include <QJsonDocument>

namespace ATProto::AppBskyGraph {

// app.bsky.graph.getFollows/output
struct GetFollowsOutput
{
    AppBskyActor::ProfileView::Ptr mSubject;
    AppBskyActor::ProfileViewList mFollows;
    std::optional<QString> mCursor;

    using Ptr = std::unique_ptr<GetFollowsOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.graph.getFollowers/output
struct GetFollowersOutput
{
    AppBskyActor::ProfileView::Ptr mSubject;
    AppBskyActor::ProfileViewList mFollowers;
    std::optional<QString> mCursor;

    using Ptr = std::unique_ptr<GetFollowersOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.graph.getBlocks/output
struct GetBlocksOutput
{
    AppBskyActor::ProfileViewList mBlocks;
    std::optional<QString> mCursor;

    using Ptr = std::unique_ptr<GetBlocksOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.graph.getMutes/output
struct GetMutesOutput
{
    AppBskyActor::ProfileViewList mMutes;
    std::optional<QString> mCursor;

    using Ptr = std::unique_ptr<GetMutesOutput>;
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

// app.bsky.graph.block
struct Block
{
    QString mSubject; // did
    QDateTime mCreatedAt;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Block>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.graph.defs#listViewerState
struct ListViewerState
{
    bool mMuted = false;
    std::optional<QString> mBlocked; // at-uri

    using Ptr = std::unique_ptr<ListViewerState>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.graph.defs#listViewBasic
struct ListViewBasic
{
    QString mUri;
    QString mCid;
    QString mName;
    QString mPurpose;
    std::optional<QString> mAvatar;
    ListViewerState::Ptr mViewer; // optional
    std::optional<QDateTime> mIndexedAt;

    using Ptr = std::unique_ptr<ListViewBasic>;
    static Ptr fromJson(const QJsonObject& json);
};

using ListViewBasicList = std::vector<ListViewBasic::Ptr>;

}
