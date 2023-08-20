// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include <QJsonDocument>

namespace ATProto::AppBskyFeed {

// Record types
namespace Record {

// app.bsky.feed.post
struct Post
{
    QString mText; // max 300 graphemes, 3000 bytes
    // TODO facets
    // TODO reply
    // TODO embed
    QDateTime mCreatedAt;

    using Ptr = std::unique_ptr<Post>;
    static Ptr fromJson(const QJsonObject& json);
};

}

// app.bsky.feed.defs#postView
struct PostView
{
    QString mUri; // at-uri
    QString mCid;
    AppBskyActor::ProfileViewBasic::Ptr mAuthor; // required
    std::variant<Record::Post::Ptr> mRecord;
    // TODO embed
    std::optional<int> mReplyCount;
    std::optional<int> mRepostCount;
    std::optional<int> mLikeCount;
    QDateTime mIndexedAt;
    // TODO viewer
    // TODO labels

    using Ptr = std::unique_ptr<PostView>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.defs#feedViewPost
struct FeedViewPost
{
    PostView::Ptr mPost; // required
    // TODO reply;
    // TODO reason;

    using Ptr = std::unique_ptr<FeedViewPost>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.getAuthorFeed#output
struct AuthorFeed
{
    std::optional<QString> mCursor;
    std::vector<FeedViewPost::Ptr> mFeed;

    using Ptr = std::unique_ptr<AuthorFeed>;
    static Ptr fromJson(const QJsonDocument& json);
};

}
