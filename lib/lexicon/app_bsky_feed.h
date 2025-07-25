// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include "app_bsky_embed.h"
#include "app_bsky_feed_include.h"
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
    bool mThreadMuted = false;
    bool mReplyDisabled = false;
    bool mEmbeddingDisabled = false;
    bool mPinned = false;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ViewerState>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.feed.postgate
struct Postgate
{
    QDateTime mCreatedAt;
    QString mPost; // at-uri
    std::vector<QString> mDetachedEmbeddingUris;
    bool mDisableEmbedding = false;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Postgate>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.feed.postgate";
};

// app.bsky.feed.threadgate
struct Threadgate
{
    QString mPost; // at-uri
    ThreadgateRules mRules;
    std::unordered_set<QString> mHiddenReplies; // at-uri list
    QDateTime mCreatedAt;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Threadgate>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.feed.threadgate";
};

// app.bsky.feed.defs#threadgateView
struct ThreadgateView
{
    // NOTE: It seems odd that all fields are optional.
    std::optional<QString> mUri;
    std::optional<QString> mCid;
    Threadgate::SharedPtr mRecord; // Can be nullptr when other record types get spec'd
    QString mRawRecordType;
    AppBskyGraph::ListViewBasicList mLists;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ThreadgateView>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.feed.defs#postView
struct PostView
{
    QString mUri; // at-uri
    QString mCid;
    AppBskyActor::ProfileViewBasic::SharedPtr mAuthor; // required
    std::variant<Record::Post::SharedPtr> mRecord;
    RecordType mRecordType;
    QString mRawRecordType;
    AppBskyEmbed::EmbedView::SharedPtr mEmbed; // optional
    int mReplyCount = 0;
    int mRepostCount = 0;
    int mLikeCount = 0;
    int mQuoteCount = 0;
    QDateTime mIndexedAt;
    ViewerState::SharedPtr mViewer;
    ComATProtoLabel::LabelList mLabels;
    ThreadgateView::SharedPtr mThreadgate; // optional

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<PostView>;
    using List = std::vector<PostView::SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.feed.defs#postView";
};

struct GetPostsOutput
{
    PostView::List mPosts;

    using SharedPtr = std::shared_ptr<GetPostsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.feed.defs#notFoundPost
struct NotFoundPost
{
    QString mUri; // at-uri

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<NotFoundPost>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.feed.defs#notFoundPost";
};

// app.bsky.feed.defs#blockedAuthor
struct BlockedAuthor
{
    QString mDid;
    // NOT IMPLEMENTED viewer

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<BlockedAuthor>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "";
};

// app.bsky.feed.defs#blockedPost
struct BlockedPost
{
    QString mUri; // at-uri
    BlockedAuthor::SharedPtr mAuthor; // required

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<BlockedPost>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.feed.defs#blockedPost";
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
    std::variant<PostView::SharedPtr, NotFoundPost::SharedPtr, BlockedPost::SharedPtr> mPost;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ReplyElement>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.feed.defs#replyRef
struct ReplyRef
{
    ReplyElement::SharedPtr mRoot; // required
    ReplyElement::SharedPtr mParent; // required
    AppBskyActor::ProfileViewBasic::SharedPtr mGrandparentAuthor; // optional

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ReplyRef>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.feed.defs#reasonRepost
struct ReasonRepost
{
    AppBskyActor::ProfileViewBasic::SharedPtr mBy;
    std::optional<QString> mUri; // at-uri
    std::optional<QString> mCid;
    QDateTime mIndexedAt;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ReasonRepost>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.feed.defs#reasonRepost";
};

// app.bsky.feed.defs#reasonPin
struct ReasonPin
{
    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ReasonPin>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.feed.defs#reasonPin";
};

// app.bsky.feed.defs#feedViewPost
struct FeedViewPost
{
    PostView::SharedPtr mPost; // required
    ReplyRef::SharedPtr mReply;
    std::optional<std::variant<ReasonRepost::SharedPtr, ReasonPin::SharedPtr>> mReason;
    std::optional<QString> mFeedContext;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<FeedViewPost>;
    static SharedPtr fromJson(const QJsonObject& json);
};

using PostFeed = std::vector<FeedViewPost::SharedPtr>;

// app.bsky.feed.getAuthorFeed#output
// app.bsky.feed.getTimeline#ouput
// app.bsky.feed.getFeed#ouput
struct OutputFeed
{
    std::optional<QString> mCursor;
    PostFeed mFeed;

    using SharedPtr = std::shared_ptr<OutputFeed>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct ThreadElement;

// app.bsky.feed.defs#threadViewPost
struct ThreadViewPost
{
    PostView::SharedPtr mPost; // required
    std::shared_ptr<ThreadElement> mParent; // optional
    std::vector<std::shared_ptr<ThreadElement>> mReplies;

    using SharedPtr = std::shared_ptr<ThreadViewPost>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct ThreadElement
{
    PostElementType mType;
    QString mUnsupportedType;
    std::variant<ThreadViewPost::SharedPtr, NotFoundPost::SharedPtr, BlockedPost::SharedPtr> mPost;

    using SharedPtr = std::shared_ptr<ThreadElement>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.feed.getPostThread/output
struct PostThread
{
    ThreadElement::SharedPtr mThread; // required
    ThreadgateView::SharedPtr mThreadgate; // optional

    using SharedPtr = std::shared_ptr<PostThread>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.feed.like
struct Like
{
    ComATProtoRepo::StrongRef::SharedPtr mSubject;
    QDateTime mCreatedAt;
    ComATProtoRepo::StrongRef::SharedPtr mVia; // optional

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Like>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.feed.like";
};

// app.bsky.feed.repost
struct Repost
{
    ComATProtoRepo::StrongRef::SharedPtr mSubject;
    QDateTime mCreatedAt;
    ComATProtoRepo::StrongRef::SharedPtr mVia; // optional

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Repost>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.feed.repost";
};

// app.bsky.feed.getLikes#like
struct GetLikesLike
{
    QDateTime mIndexedAt;
    QDateTime mCreatedAt;
    AppBskyActor::ProfileView::SharedPtr mActor;

    using SharedPtr = std::shared_ptr<GetLikesLike>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.feed.getLikes/output
struct GetLikesOutput
{
    QString mUri;
    std::optional<QString> mCid;
    std::vector<GetLikesLike::SharedPtr> mLikes;
    std::optional<QString> mCursor;

    using SharedPtr = std::shared_ptr<GetLikesOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct SearchSortOrder : public QObject
{
    Q_OBJECT
    QML_UNCREATABLE("Class only exposes constants to QML.")
    QML_ELEMENT
    QML_SINGLETON

public:
    SHARED_CONST(QString, TOP, QStringLiteral("top"));
    SHARED_CONST(QString, LATEST, QStringLiteral("latest"));
};

struct AuthorFeedFilter : public QObject
{
    Q_OBJECT
    QML_UNCREATABLE("Class only exposes constants to QML.")
    QML_ELEMENT
    QML_SINGLETON

public:
    SHARED_CONST(QString, POSTS_WITH_REPLIES, QStringLiteral("posts_with_replies"));
    SHARED_CONST(QString, POSTS_NO_REPLIES, QStringLiteral("posts_no_replies"));
    SHARED_CONST(QString, POSTS_WITH_MEDIA, QStringLiteral("posts_with_media"));
    SHARED_CONST(QString, POSTS_AND_AUTHOR_THREADS, QStringLiteral("posts_and_author_threads"));
    SHARED_CONST(QString, POSTS_WITH_VIDEO, QStringLiteral("posts_with_video"));
};

// app.bsky.feed.getRepostedBy/Ouput
struct GetRepostedByOutput
{
    QString mUri;
    std::optional<QString> mCid;
    AppBskyActor::ProfileViewList mRepostedBy;
    std::optional<QString> mCursor;

    using SharedPtr = std::shared_ptr<GetRepostedByOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.feed.searchPosts#output
struct SearchPostsOutput
{
    std::optional<QString> mCursor;
    std::optional<int> mHitsTotal;
    PostView::List mPosts;

    using SharedPtr = std::shared_ptr<SearchPostsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct GetFeedGeneratorOutput
{
    GeneratorView::SharedPtr mView; // required
    bool mIsOnline;
    bool mIsValid;

    using SharedPtr = std::shared_ptr<GetFeedGeneratorOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct GetFeedGeneratorsOutput
{
    GeneratorViewList mFeeds;

    using SharedPtr = std::shared_ptr<GetFeedGeneratorsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct GetActorFeedsOutput
{
    GeneratorViewList mFeeds;
    std::optional<QString> mCursor;

    using SharedPtr = std::shared_ptr<GetActorFeedsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct GetQuotesOutput
{
    QString mUri; // at-uri
    std::optional<QString> mCid;
    std::optional<QString> mCursor;
    PostView::List mPosts;

    using SharedPtr = std::shared_ptr<GetQuotesOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.feed.defs#interaction
struct Interaction
{
    enum class EventType
    {
        RequestLess,
        RequestMore
    };
    static QString eventTypeToString(EventType eventType);

    std::optional<QString> mItem; // at-uri
    std::optional<EventType> mEvent;
    std::optional<QString> mFeedContext;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Interaction>;
};

using InteractionList = std::vector<Interaction::SharedPtr>;

}
