// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include "app_bsky_embed.h"
#include "app_bsky_graph.h"
#include "com_atproto_label.h"
#include "lexicon.h"
#include <QJsonDocument>

namespace ATProto::AppBskyFeed {

// app.bsky.feed.defs#viewerState
struct ViewerState
{
    std::optional<QString> mRepost;
    std::optional<QString> mLike;
    bool mReplyDisabled = false;

    using Ptr = std::unique_ptr<ViewerState>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.threadgate#listRule
struct ThreadgateListRule
{
    QString mList; // at-uri

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<ThreadgateListRule>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.threadgate
struct Threadgate
{
    QString mPost; // at-uri
    bool mAllowMention = false;
    bool mAllowFollowing = false;
    std::vector<ThreadgateListRule::Ptr> mAllowList;
    QDateTime mCreatedAt;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Threadgate>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.defs#threadgateView
struct ThreadgateView
{
    // NOTE: It seems odd that all fields are optional.
    std::optional<QString> mUri;
    std::optional<QString> mCid;
    Threadgate::Ptr mRecord; // Can be nullptr when other record types get spec'd
    QString mRawRecordType;
    AppBskyGraph::ListViewBasicList mLists;

    using Ptr = std::unique_ptr<ThreadgateView>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.defs#postView
struct PostView
{
    QString mUri; // at-uri
    QString mCid;
    AppBskyActor::ProfileViewBasic::Ptr mAuthor; // required
    std::variant<Record::Post::Ptr> mRecord;
    RecordType mRecordType;
    QString mRawRecordType;
    AppBskyEmbed::EmbedView::Ptr mEmbed; // optional
    int mReplyCount = 0;
    int mRepostCount = 0;
    int mLikeCount = 0;
    QDateTime mIndexedAt;
    ViewerState::Ptr mViewer;
    std::vector<ComATProtoLabel::Label::Ptr> mLabels;
    ThreadgateView::Ptr mThreadgate; // optional

    using SharedPtr = std::shared_ptr<PostView>;
    using Ptr = std::unique_ptr<PostView>;
    static Ptr fromJson(const QJsonObject& json);
};

using PostViewList = std::vector<PostView::Ptr>;
void getPostViewList(PostViewList& list, const QJsonObject& json);

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

enum class PostElementType
{
    NOT_FOUND_POST,
    BLOCKED_POST,
    THREAD_VIEW_POST,
    POST_VIEW,
    UNKNOWN
};
PostElementType stringToPostElementType(const QString& str);

struct ReplyElement
{
    PostElementType mType;
    QString mUnsupportedType;
    std::variant<PostView::Ptr, NotFoundPost::Ptr, BlockedPost::Ptr> mPost;

    using Ptr = std::unique_ptr<ReplyElement>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.defs#replyRef
struct ReplyRef
{
    ReplyElement::Ptr mRoot; // required
    ReplyElement::Ptr mParent; // required

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
// app.bsky.feed.getFeed#ouput
struct OutputFeed
{
    std::optional<QString> mCursor;
    PostFeed mFeed;

    using Ptr = std::unique_ptr<OutputFeed>;
    static Ptr fromJson(const QJsonDocument& json);
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

struct ThreadElement
{
    PostElementType mType;
    QString mUnsupportedType;
    std::variant<ThreadViewPost::Ptr, NotFoundPost::Ptr, BlockedPost::Ptr> mPost;

    using Ptr = std::unique_ptr<ThreadElement>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.getPostThread/output
struct PostThread
{
    ThreadElement::Ptr mThread; // required

    using Ptr = std::unique_ptr<PostThread>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.like
struct Like
{
    ComATProtoRepo::StrongRef::Ptr mSubject;
    QDateTime mCreatedAt;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Like>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.repost
struct Repost
{
    ComATProtoRepo::StrongRef::Ptr mSubject;
    QDateTime mCreatedAt;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Repost>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.getLikes#like
struct GetLikesLike
{
    QDateTime mIndexedAt;
    QDateTime mCreatedAt;
    AppBskyActor::ProfileView::Ptr mActor;

    using Ptr = std::unique_ptr<GetLikesLike>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.getLikes/output
struct GetLikesOutput
{
    QString mUri;
    std::optional<QString> mCid;
    std::vector<GetLikesLike::Ptr> mLikes;
    std::optional<QString> mCursor;

    using Ptr = std::unique_ptr<GetLikesOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.getRepostedBy/Ouput
struct GetRepostedByOutput
{
    QString mUri;
    std::optional<QString> mCid;
    AppBskyActor::ProfileViewList mRepostedBy;
    std::optional<QString> mCursor;

    using Ptr = std::unique_ptr<GetRepostedByOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.searchPosts#output
struct SearchPostsOutput
{
    std::optional<QString> mCursor;
    std::optional<int> mHitsTotal;
    PostViewList mPosts;

    using Ptr = std::unique_ptr<SearchPostsOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

// Temporary legacy search till app.bsky.feed.searchPosts is supported by bsky
// https://search.bsky.social/search/posts?q=
// https://github.com/bluesky-social/social-app/blob/7ebf1ed3710081f27f90eaae125c7315798d56e5/src/lib/api/search.ts#L41
struct LegacySearchPostsOutput
{
    std::vector<QString> mUris;

    using Ptr = std::unique_ptr<LegacySearchPostsOutput>;
    static Ptr fromJson(const QJsonArray& jsonArray);
};

// app.bsky.feed.defs#generatorViewerState
struct GeneratorViewerState
{
    std::optional<QString> mLike; // at-uri

    using Ptr = std::unique_ptr<GeneratorViewerState>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.defs#generatorView
struct GeneratorView {
    QString mUri;
    QString mCid;
    QString mDid;
    AppBskyActor::ProfileView::Ptr mCreator; // required
    QString mDisplayName;
    std::optional<QString> mDescription; // max 300 graphemes, 3000 bytes
    AppBskyRichtext::FacetList mDescriptionFacets;
    std::optional<QString> mAvatar;
    int mLikeCount = 0;
    GeneratorViewerState::Ptr mViewer; // optional
    QDateTime mIndexedAt;

    using SharedPtr = std::shared_ptr<GeneratorView>;
    using Ptr = std::unique_ptr<GeneratorView>;
    static Ptr fromJson(const QJsonObject& json);
};
using GeneratorViewList = std::vector<GeneratorView::Ptr>;

struct GetFeedGeneratorOutput
{
    GeneratorView::Ptr mView; // required
    bool mIsOnline;
    bool mIsValid;

    using Ptr = std::unique_ptr<GetFeedGeneratorOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

struct GetFeedGeneratorsOutput
{
    GeneratorViewList mFeeds;

    using Ptr = std::unique_ptr<GetFeedGeneratorsOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

}
