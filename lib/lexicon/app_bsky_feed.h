// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include "app_bsky_embed.h"
#include "com_atproto_label.h"
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
    int mReplyCount = 0;
    int mRepostCount = 0;
    int mLikeCount = 0;
    QDateTime mIndexedAt;
    // NOT IMPLEMENTED viewer
    std::vector<ComATProtoLabel::Label::Ptr> mLabels;

    using Ptr = std::unique_ptr<PostView>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.defs#replyRef
struct ReplyRef
{
    // TODO: notFound, blocked
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

using PostFeed = std::vector<FeedViewPost::Ptr>;

// app.bsky.feed.getAuthorFeed#output
// app.bsky.feed.getTimeline#ouput
struct OutputFeed
{
    std::optional<QString> mCursor;
    PostFeed mFeed;

    using Ptr = std::unique_ptr<OutputFeed>;
    static Ptr fromJson(const QJsonDocument& json);
};

// app.bsky.feed.defs#notFoundPost
struct NotFoundPost
{
    QString mUri; // at-uri

    using Ptr = std::unique_ptr<NotFoundPost>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.defs#blockedAuthor
struct BlockedAuthor
{
    QString mDid;
    // NOT IMPLEMENTED viewer

    using Ptr = std::unique_ptr<BlockedAuthor>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.defs#blockedPost
struct BlockedPost
{
    QString mUri; // at-uri
    BlockedAuthor::Ptr mAuthor; // required

    using Ptr = std::unique_ptr<BlockedPost>;
    static Ptr fromJson(const QJsonObject& json);
};

struct ThreadElement;

// app.bsky.feed.defs#threadViewPost
struct ThreadViewPost
{
    PostView::Ptr mPost; // required
    std::unique_ptr<ThreadElement> mParent; // optional
    std::vector<std::unique_ptr<ThreadElement>> mReplies;

    using Ptr = std::unique_ptr<ThreadViewPost>;
    static Ptr fromJson(const QJsonObject& json);
};

enum class ThreadElementType
{
    NOT_FOUND_POST,
    BLOCKED_POST,
    THREAD_VIEW_POST,
    UNKNOWN
};
ThreadElementType stringToThreadElementType(const QString& str);

struct ThreadElement
{
    ThreadElementType mType;
    std::variant<ThreadViewPost::Ptr, NotFoundPost::Ptr, BlockedPost::Ptr> mPost;

    using Ptr = std::unique_ptr<ThreadElement>;
    static Ptr fromJson(const QJsonObject& json);
};

}
