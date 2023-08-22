// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include "app_bsky_embed.h"
#include "lexicon.h"
#include <QJsonDocument>

namespace ATProto::AppBskyFeed {

// app.bsky.feed.defs#postView
struct PostView
{
    QString mUri; // at-uri
    QString mCid;
    AppBskyActor::ProfileViewBasic::Ptr mAuthor; // required
    std::variant<Record::Post::Ptr> mRecord;
    RecordType mRecordType;
    AppBskyEmbed::Embed::Ptr mEmbed; // optional
    std::optional<int> mReplyCount;
    std::optional<int> mRepostCount;
    std::optional<int> mLikeCount;
    QDateTime mIndexedAt;
    // TODO viewer
    // TODO labels

    using Ptr = std::unique_ptr<PostView>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.defs#replyRef
struct ReplyRef
{
    PostView::Ptr mRoot; // required
    PostView::Ptr mParent; // required

    using Ptr = std::unique_ptr<ReplyRef>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.defs#reasonRepost
struct ReasonRepost
{
    AppBskyActor::ProfileViewBasic::Ptr mBy;
    QDateTime mIndexedAt;

    using Ptr = std::unique_ptr<ReasonRepost>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.defs#feedViewPost
struct FeedViewPost
{
    PostView::Ptr mPost; // required
    ReplyRef::Ptr mReply;
    ReasonRepost::Ptr mReason;

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
