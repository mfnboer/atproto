// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_feed.h"
#include "app_bsky_actor.h"
#include "../xjson.h"
#include <QJsonArray>
#include <unordered_map>

namespace ATProto::AppBskyFeed {

QJsonObject ViewerState::toJson() const
{
    QJsonObject json;
    XJsonObject::insertOptionalJsonValue(json, "repost", mRepost);
    XJsonObject::insertOptionalJsonValue(json, "like", mLike);
    XJsonObject::insertOptionalJsonValue(json, "threadMuted", mThreadMuted, false);
    XJsonObject::insertOptionalJsonValue(json, "replyDisabled", mReplyDisabled, false);
    XJsonObject::insertOptionalJsonValue(json, "embeddingDisabled", mEmbeddingDisabled,false);
    XJsonObject::insertOptionalJsonValue(json, "pinned", mPinned, false);
    return json;
}

ViewerState::SharedPtr ViewerState::fromJson(const QJsonObject& json)
{
    auto viewerState = std::make_shared<ViewerState>();
    XJsonObject xjson(json);
    viewerState->mRepost = xjson.getOptionalString("repost");
    viewerState->mLike = xjson.getOptionalString("like");
    viewerState->mThreadMuted = xjson.getOptionalBool("threadMuted", false);
    viewerState->mReplyDisabled = xjson.getOptionalBool("replyDisabled", false);
    viewerState->mEmbeddingDisabled = xjson.getOptionalBool("embeddingDisabled", false);
    viewerState->mPinned = xjson.getOptionalBool("pinned", false);
    return viewerState;
}

QJsonObject PostgateDisableRule::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    return json;
}

PostgateDisableRule::SharedPtr PostgateDisableRule::fromJson(const QJsonObject&)
{
    auto rule = std::make_shared<PostgateDisableRule>();
    return rule;
}

void PostgateEmbeddingRules::insertDisableEmbedding(QJsonObject& json, const QString& field, bool disableEmbedding)
{
    std::vector<RuleType> rules;

    if (disableEmbedding)
        rules.push_back(std::make_shared<PostgateDisableRule>());

    XJsonObject::insertOptionalVariantArray(json, field, rules);
}

bool PostgateEmbeddingRules::getDisableEmbedding(const QJsonObject& json, const QString& field)
{
    XJsonObject xjson(json);

    std::vector<RuleType> rules = xjson.getOptionalVariantList<PostgateDisableRule>(field);

    for (const auto& rule : rules)
    {
        if (std::holds_alternative<PostgateDisableRule::SharedPtr>(rule))
            return true;
    }

    return false;
}

QJsonObject Postgate::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));
    json.insert("post", mPost);
    XJsonObject::insertOptionalArray(json, "detachedEmbeddingUris", mDetachedEmbeddingUris);
    PostgateEmbeddingRules::insertDisableEmbedding(json, "embeddingRules", mDisableEmbedding);

    return json;
}

Postgate::SharedPtr Postgate::fromJson(const QJsonObject& json)
{
    auto postgate = std::make_shared<Postgate>();
    XJsonObject xjson(json);
    postgate->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    postgate->mPost = xjson.getRequiredString("post");
    postgate->mDetachedEmbeddingUris = xjson.getOptionalStringVector("detachedEmbeddingUris");
    postgate->mDisableEmbedding = PostgateEmbeddingRules::getDisableEmbedding(json, "embeddingRules");
    return postgate;
}

QJsonObject ThreadgateListRule::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("list", mList);
    return json;
}

ThreadgateListRule::SharedPtr ThreadgateListRule::fromJson(const QJsonObject& json)
{
    auto rule = std::make_shared<ThreadgateListRule>();
    XJsonObject xjson(json);
    rule->mList = xjson.getRequiredString("list");
    return rule;
}

QJsonArray ThreadgateRules::toJson() const
{
    QJsonArray allowArray;

    if (mAllowMention)
    {
        QJsonObject rule;
        rule.insert("$type", "app.bsky.feed.threadgate#mentionRule");
        allowArray.append(rule);
    }

    if (mAllowFollower)
    {
        QJsonObject rule;
        rule.insert("$type", "app.bsky.feed.threadgate#followerRule");
        allowArray.append(rule);
    }

    if (mAllowFollowing)
    {
        QJsonObject rule;
        rule.insert("$type", "app.bsky.feed.threadgate#followingRule");
        allowArray.append(rule);
    }

    for (const auto& listRule : mAllowList)
        allowArray.append(listRule->toJson());

    return allowArray;
}

ThreadgateRules::SharedPtr ThreadgateRules::fromJson(const QJsonArray& allowArray)
{
    auto threadgateRules = std::make_shared<ThreadgateRules>();

    for (const auto& allowElem : allowArray)
    {
        if (!allowElem.isObject())
        {
            qWarning() << "PROTO ERROR invalid threadgate allow element: not an object";
            qInfo() << allowArray;
            throw InvalidJsonException("PROTO ERROR invalid threadgate element: allow");
        }

        auto allowJson = allowElem.toObject();
        XJsonObject allowXJson(allowJson);
        QString type = allowXJson.getRequiredString("$type");

        if (type == "app.bsky.feed.threadgate#mentionRule")
        {
            threadgateRules->mAllowMention = true;
        }
        else if (type == "app.bsky.feed.threadgate#followerRule")
        {
            threadgateRules->mAllowFollower = true;
        }
        else if (type == "app.bsky.feed.threadgate#followingRule")
        {
            threadgateRules->mAllowFollowing = true;
        }
        else if (type == "app.bsky.feed.threadgate#listRule")
        {
            auto listRule = ThreadgateListRule::fromJson(allowJson);
            threadgateRules->mAllowList.push_back(std::move(listRule));
        }
        else
        {
            qWarning() << "Unknown threadgate rule type:" << type;
        }
    }

    return threadgateRules;
}

QJsonObject Threadgate::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("post", mPost);

    QJsonArray allowArray = mRules.toJson();

    if (!allowArray.isEmpty() || mRules.mAllowNobody)
        json.insert("allow", allowArray);

    std::vector<QString> replies(mHiddenReplies.begin(), mHiddenReplies.end());
    XJsonObject::insertOptionalArray(json, "hiddenReplies", replies);

    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));
    return json;
}

Threadgate::SharedPtr Threadgate::fromJson(const QJsonObject& json)
{
    auto threadgate = std::make_shared<Threadgate>();
    XJsonObject xjson(json);
    threadgate->mPost = xjson.getRequiredString("post");
    const auto allowArray = xjson.getOptionalArray("allow");

    if (allowArray)
    {
        const auto rules = ThreadgateRules::fromJson(*allowArray);
        threadgate->mRules = *rules;
    }

    const auto replies = xjson.getOptionalStringVector("hiddenReplies");
    threadgate->mHiddenReplies.insert(replies.begin(), replies.end());

    // Initially the hidden replies did not exist and an empty threadgate was interpreted
    // as nobody.
    threadgate->mRules.mAllowNobody = (allowArray && allowArray->isEmpty()) || (!allowArray && threadgate->mHiddenReplies.empty());

    threadgate->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return threadgate;
}

QJsonObject ThreadgateView::toJson() const
{
    QJsonObject json;
    XJsonObject::insertOptionalJsonValue(json, "uri", mUri);
    XJsonObject::insertOptionalJsonValue(json, "cid", mCid);
    XJsonObject::insertOptionalJsonObject<Threadgate>(json, "record", mRecord);
    XJsonObject::insertOptionalArray<AppBskyGraph::ListViewBasic>(json, "lists", mLists);
    return json;
}

ThreadgateView::SharedPtr ThreadgateView::fromJson(const QJsonObject& json)
{
    auto threadgateView = std::make_shared<ThreadgateView>();
    XJsonObject xjson(json);
    threadgateView->mUri = xjson.getOptionalString("uri");
    threadgateView->mCid = xjson.getOptionalString("cid");
    auto recordJson = xjson.getOptionalJsonObject("record");

    if (recordJson)
    {
        XJsonObject recordXJson(*recordJson);
        threadgateView->mRawRecordType = recordXJson.getRequiredString("$type");

        if (threadgateView->mRawRecordType == "app.bsky.feed.threadgate")
            threadgateView->mRecord = Threadgate::fromJson(*recordJson);
        else
            qWarning() << "Unknow threadgate view record type:" << threadgateView->mRawRecordType;
    }

    threadgateView->mLists = xjson.getOptionalVector<AppBskyGraph::ListViewBasic>("lists");
    return threadgateView;
}

QJsonObject PostReplyRef::toJson() const
{
    QJsonObject json;
    json.insert("root", mRoot->toJson());
    json.insert("parent", mParent->toJson());
    return json;
}

PostReplyRef::SharedPtr PostReplyRef::fromJson(const QJsonObject& json)
{
    auto replyRef = std::make_shared<PostReplyRef>();
    XJsonObject xjson(json);
    replyRef->mRoot = xjson.getRequiredObject<ComATProtoRepo::StrongRef>("root");
    replyRef->mParent = xjson.getRequiredObject<ComATProtoRepo::StrongRef>("parent");
    return replyRef;
}

QJsonObject Record::Post::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", "app.bsky.feed.post");
    json.insert("text", mText);
    XJsonObject::insertOptionalArray<AppBskyRichtext::Facet>(json, "facets", mFacets);
    XJsonObject::insertOptionalJsonObject<PostReplyRef>(json, "reply", mReply);
    XJsonObject::insertOptionalJsonObject<AppBskyEmbed::Embed>(json, "embed", mEmbed);
    XJsonObject::insertOptionalJsonObject<ComATProtoLabel::SelfLabels>(json, "labels", mLabels);

    if (!mLanguages.empty())
        json.insert("langs", XJsonObject::toJsonArray(mLanguages));

    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));
    XJsonObject::insertOptionalJsonValue(json, "bridgyOriginalText", mBridgyOriginalText);
    return json;
}

Record::Post::SharedPtr Record::Post::fromJson(const QJsonObject& json)
{
    auto post = std::make_shared<Record::Post>();
    XJsonObject xjson(json);
    post->mText = xjson.getRequiredString("text");
    post->mFacets = xjson.getOptionalVector<AppBskyRichtext::Facet>("facets");
    post->mReply = xjson.getOptionalObject<PostReplyRef>("reply");
    post->mEmbed = xjson.getOptionalObject<AppBskyEmbed::Embed>("embed");
    post->mLabels = xjson.getOptionalObject<ComATProtoLabel::SelfLabels>("labels");
    post->mLanguages = xjson.getOptionalStringVector("langs");
    post->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    post->mBridgyOriginalText = xjson.getOptionalString("bridgyOriginalText");
    post->mJson = json;
    return post;
}

QJsonObject PostView::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("uri", mUri);
    json.insert("cid", mCid);
    XJsonObject::insertOptionalJsonObject<AppBskyActor::ProfileViewBasic>(json, "author", mAuthor);
    json.insert("record", XJsonObject::variantToJsonObject(mRecord));
    XJsonObject::insertOptionalJsonObject<AppBskyEmbed::EmbedView>(json, "embed", mEmbed);
    XJsonObject::insertOptionalJsonValue(json, "replyCount", mReplyCount, 0);
    XJsonObject::insertOptionalJsonValue(json, "repostCount", mRepostCount, 0);
    XJsonObject::insertOptionalJsonValue(json, "likeCount", mLikeCount, 0);
    XJsonObject::insertOptionalJsonValue(json, "quoteCount", mQuoteCount, 0);
    json.insert("indexedAt", mIndexedAt.toString(Qt::ISODateWithMs));
    XJsonObject::insertOptionalJsonObject<ViewerState>(json, "viewer", mViewer);
    XJsonObject::insertOptionalArray<ComATProtoLabel::Label>(json, "labels", mLabels);
    XJsonObject::insertOptionalJsonObject<ThreadgateView>(json, "threadgate", mThreadgate);

    return json;
}

PostView::SharedPtr PostView::fromJson(const QJsonObject& json)
{
    auto postView = std::make_shared<PostView>();
    XJsonObject xjson(json);
    postView->mUri = xjson.getRequiredString("uri");
    postView->mCid = xjson.getRequiredString("cid");
    postView->mAuthor = AppBskyActor::ProfileViewBasic::fromJson(json["author"].toObject());
    const auto recordObj = xjson.getRequiredJsonObject("record");
    const XJsonObject record(recordObj);
    postView->mRawRecordType = record.getRequiredString("$type");
    postView->mRecordType = stringToRecordType(postView->mRawRecordType);

    if (postView->mRecordType == RecordType::APP_BSKY_FEED_POST)
        postView->mRecord = Record::Post::fromJson(record.getObject());
    else
        qWarning() << QString("Unsupported record type in app.bsky.feed.defs#postView: %1").arg(postView->mRawRecordType);
    
    postView->mEmbed = xjson.getOptionalObject<AppBskyEmbed::EmbedView>("embed");
    postView->mReplyCount = xjson.getOptionalInt("replyCount", 0);
    postView->mRepostCount = xjson.getOptionalInt("repostCount", 0);
    postView->mLikeCount = xjson.getOptionalInt("likeCount", 0);
    postView->mQuoteCount = xjson.getOptionalInt("quoteCount", 0);
    postView->mIndexedAt = xjson.getRequiredDateTime("indexedAt");
    postView->mViewer = xjson.getOptionalObject<ViewerState>("viewer");
    ComATProtoLabel::getLabels(postView->mLabels, json);
    postView->mThreadgate = xjson.getOptionalObject<ThreadgateView>("threadgate");
    return postView;
}

GetPostsOutput::SharedPtr GetPostsOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<GetPostsOutput>();
    output->mPosts = xjson.getRequiredVector<PostView>("posts");
    return output;
}

QJsonObject ReplyElement::toJson() const
{
    return XJsonObject::variantToJsonObject(mPost);
}

ReplyElement::SharedPtr ReplyElement::fromJson(const QJsonObject& json)
{
    auto element = std::make_shared<ReplyElement>();
    const XJsonObject xjson(json);
    const auto typeString = xjson.getRequiredString("$type");
    element->mType = stringToPostElementType(typeString);

    switch (element->mType)
    {
    case PostElementType::NOT_FOUND_POST:
        element->mPost = NotFoundPost::fromJson(json);
        break;
    case PostElementType::BLOCKED_POST:
        element->mPost = BlockedPost::fromJson(json);
        break;
    case PostElementType::POST_VIEW:
        element->mPost = PostView::fromJson(json);
        break;
    case PostElementType::THREAD_VIEW_POST:
    case PostElementType::UNKNOWN:
        qWarning() << "Unsupported thread element type:" << typeString << "json:" << json;
        element->mUnsupportedType = typeString;
        break;
    }

    return element;
}

QJsonObject ReplyRef::toJson() const
{
    QJsonObject json;
    json.insert("root", mRoot->toJson());
    json.insert("parent", mParent->toJson());
    XJsonObject::insertOptionalJsonObject<AppBskyActor::ProfileViewBasic>(json, "grandparentAuthor", mGrandparentAuthor);
    return json;
}

ReplyRef::SharedPtr ReplyRef::fromJson(const QJsonObject& json)
{
    auto replyRef = std::make_shared<ReplyRef>();
    XJsonObject xjson(json);
    replyRef->mRoot = xjson.getRequiredObject<ReplyElement>("root");
    replyRef->mParent = xjson.getRequiredObject<ReplyElement>("parent");
    replyRef->mGrandparentAuthor = xjson.getOptionalObject<AppBskyActor::ProfileViewBasic>("grandparentAuthor");
    return replyRef;
}


QJsonObject ReasonRepost::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("by", mBy->toJson());
    json.insert("indexedAt", mIndexedAt.toString(Qt::ISODateWithMs));
    return json;
}

ReasonRepost::SharedPtr ReasonRepost::fromJson(const QJsonObject& json)
{
    auto reason = std::make_shared<ReasonRepost>();
    XJsonObject xjson(json);
    reason->mBy = xjson.getRequiredObject<AppBskyActor::ProfileViewBasic>("by");
    reason->mUri = xjson.getOptionalString("uri");
    reason->mCid = xjson.getOptionalString("cid");
    reason->mIndexedAt = xjson.getRequiredDateTime("indexedAt");
    return reason;
}

QJsonObject ReasonPin::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    return json;
}

ReasonPin::SharedPtr ReasonPin::fromJson(const QJsonObject&)
{
    auto reason = std::make_shared<ReasonPin>();
    return reason;
}

QJsonObject FeedViewPost::toJson() const
{
    QJsonObject json;
    json.insert("post", mPost->toJson());
    XJsonObject::insertOptionalJsonObject<ReplyRef>(json, "reply", mReply);
    XJsonObject::insertOptionalVariant(json, "reason", mReason);
    XJsonObject::insertOptionalJsonValue(json, "feedContext", mFeedContext);
    return json;
}

FeedViewPost::SharedPtr FeedViewPost::fromJson(const QJsonObject& json)
{
    auto feedViewPost = std::make_shared<FeedViewPost>();
    XJsonObject xjson(json);
    feedViewPost->mPost = xjson.getRequiredObject<PostView>("post");
    feedViewPost->mReply = xjson.getOptionalObject<ReplyRef>("reply");
    feedViewPost->mReason = xjson.getOptionalVariant<ReasonRepost, ReasonPin>("reason");
    feedViewPost->mFeedContext = xjson.getOptionalString("feedContext");
    return feedViewPost;
}

static void getFeed(PostFeed& feed, const QJsonObject& json)
{
    XJsonObject xjson(json);

    try {
        const QJsonArray& feedArray = xjson.getRequiredArray("feed");
        feed.reserve(feedArray.size());

        for (const auto& feedJson : feedArray)
        {
            if (!feedJson.isObject())
            {
                qWarning() << "PROTO ERROR invalid feed element: not an object";
                qInfo() << json;
                continue;
            }

            try {
                auto feedViewPost = FeedViewPost::fromJson(feedJson.toObject());
                feed.push_back(std::move(feedViewPost));
            } catch (InvalidJsonException& e) {
                qWarning() << "PROTO ERROR invalid feed element:" << e.msg();
                qInfo() << json;
                continue;
            }
        }
    } catch (InvalidJsonException& e) {
        qWarning() << "PROTO ERROR invalid feed:" << e.msg();
        qInfo() << json;
    }
}

OutputFeed::SharedPtr OutputFeed::fromJson(const QJsonObject& json)
{
    auto outputFeed = std::make_shared<OutputFeed>();
    XJsonObject xjson(json);
    outputFeed->mCursor = xjson.getOptionalString("cursor");
    getFeed(outputFeed->mFeed, json);
    return outputFeed;
}

QJsonObject NotFoundPost::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("uri", mUri);
    return json;
}

NotFoundPost::SharedPtr NotFoundPost::fromJson(const QJsonObject& json)
{
    auto notFound = std::make_shared<NotFoundPost>();
    const XJsonObject xjson(json);
    notFound->mUri = xjson.getRequiredString("uri");
    return notFound;
}

QJsonObject BlockedAuthor::toJson() const
{
    QJsonObject json;
    json.insert("did", mDid);
    return json;
}

BlockedAuthor::SharedPtr BlockedAuthor::fromJson(const QJsonObject& json)
{
    auto blockedAuthor = std::make_shared<BlockedAuthor>();
    const XJsonObject xjson(json);
    blockedAuthor->mDid = xjson.getRequiredString("did");
    return blockedAuthor;
}

QJsonObject BlockedPost::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("uri", mUri);
    json.insert("author", mAuthor->toJson());
    return json;
}

BlockedPost::SharedPtr BlockedPost::fromJson(const QJsonObject& json)
{
    auto blockedPost = std::make_shared<BlockedPost>();
    const XJsonObject xjson(json);
    blockedPost->mUri = xjson.getRequiredString("uri");
    blockedPost->mAuthor = xjson.getRequiredObject<BlockedAuthor>("author");
    return blockedPost;
}

ThreadViewPost::SharedPtr ThreadViewPost::fromJson(const QJsonObject& json)
{
    auto thread = std::make_shared<ThreadViewPost>();
    const XJsonObject xjson(json);
    thread->mPost = xjson.getRequiredObject<PostView>("post");
    thread->mParent = xjson.getOptionalObject<ThreadElement>("parent");
    thread->mReplies = xjson.getOptionalVector<ThreadElement>("replies");
    return thread;
}

PostElementType stringToPostElementType(const QString& str)
{
    static const std::unordered_map<QString, PostElementType> mapping = {
        { PostView::TYPE, PostElementType::POST_VIEW },
        { "app.bsky.feed.defs#threadViewPost", PostElementType::THREAD_VIEW_POST },
        { NotFoundPost::TYPE, PostElementType::NOT_FOUND_POST },
        { BlockedPost::TYPE, PostElementType::BLOCKED_POST }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    return PostElementType::UNKNOWN;
}

ThreadElement::SharedPtr ThreadElement::fromJson(const QJsonObject& json)
{
    auto element = std::make_shared<ThreadElement>();
    const XJsonObject xjson(json);
    const auto typeString = xjson.getRequiredString("$type");
    element->mType = stringToPostElementType(typeString);

    switch (element->mType)
    {
    case PostElementType::NOT_FOUND_POST:
        element->mPost = NotFoundPost::fromJson(json);
        break;
    case PostElementType::BLOCKED_POST:
        element->mPost = BlockedPost::fromJson(json);
        break;
    case PostElementType::THREAD_VIEW_POST:
        element->mPost = ThreadViewPost::fromJson(json);
        break;
    case PostElementType::POST_VIEW:
    case PostElementType::UNKNOWN:
        qWarning() << "Unsupported thread element type:" << typeString << "json:" << json;
        element->mUnsupportedType = typeString;
        break;
    }

    return element;
}

PostThread::SharedPtr PostThread::fromJson(const QJsonObject& json)
{
    auto postThread = std::make_shared<PostThread>();
    const XJsonObject xjson(json);
    postThread->mThread = xjson.getRequiredObject<ThreadElement>("thread");
    postThread->mThreadgate = xjson.getOptionalObject<ThreadgateView>("threadgate");
    return postThread;
}

QJsonObject Like::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("subject", mSubject->toJson());
    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));
    XJsonObject::insertOptionalJsonObject<ComATProtoRepo::StrongRef>(json, "via", mVia);
    return json;
}

Like::SharedPtr Like::fromJson(const QJsonObject& json)
{
    auto like = std::make_shared<Like>();
    const XJsonObject xjson(json);
    like->mSubject = xjson.getRequiredObject<ComATProtoRepo::StrongRef>("subject");
    like->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    like->mVia = xjson.getOptionalObject<ComATProtoRepo::StrongRef>("via");
    return like;
}

QJsonObject Repost::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("subject", mSubject->toJson());
    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));
    XJsonObject::insertOptionalJsonObject<ComATProtoRepo::StrongRef>(json, "via", mVia);
    return json;
}

Repost::SharedPtr Repost::fromJson(const QJsonObject& json)
{
    auto repost = std::make_shared<Repost>();
    const XJsonObject xjson(json);
    repost->mSubject = xjson.getRequiredObject<ComATProtoRepo::StrongRef>("subject");
    repost->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    repost->mVia = xjson.getOptionalObject<ComATProtoRepo::StrongRef>("via");
    return repost;
}

GetLikesLike::SharedPtr GetLikesLike::fromJson(const QJsonObject& json)
{
    auto like = std::make_shared<GetLikesLike>();
    const XJsonObject xjson(json);
    like->mIndexedAt = xjson.getRequiredDateTime("indexedAt");
    like->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    like->mActor = xjson.getRequiredObject<AppBskyActor::ProfileView>("actor");
    return like;
}

GetLikesOutput::SharedPtr GetLikesOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetLikesOutput>();
    const XJsonObject xjson(json);
    output->mUri = xjson.getRequiredString("uri");
    output->mCid = xjson.getOptionalString("cid");
    output->mCursor = xjson.getOptionalString("cursor");
    output->mLikes = xjson.getRequiredVector<GetLikesLike>("likes");
    return output;
}

GetRepostedByOutput::SharedPtr GetRepostedByOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetRepostedByOutput>();
    const XJsonObject xjson(json);
    output->mUri = xjson.getRequiredString("uri");
    output->mCid = xjson.getOptionalString("cid");
    output->mCursor = xjson.getOptionalString("cursor");
    output->mRepostedBy = xjson.getRequiredVector<AppBskyActor::ProfileView>("repostedBy");
    return output;
}

SearchPostsOutput::SharedPtr SearchPostsOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<SearchPostsOutput>();
    const XJsonObject xjson(json);
    output->mCursor = xjson.getOptionalString("cursor");
    output->mHitsTotal = xjson.getOptionalInt("hitsTotal");
    output->mPosts = xjson.getRequiredVector<PostView>("posts");
    return output;
}

QJsonObject GeneratorViewerState::toJson() const
{
    QJsonObject json;
    XJsonObject::insertOptionalJsonValue(json, "like", mLike);
    return json;
}

GeneratorViewerState::SharedPtr GeneratorViewerState::fromJson(const QJsonObject& json)
{
    auto viewerState = std::make_shared<GeneratorViewerState>();
    const XJsonObject xjson(json);
    viewerState->mLike = xjson.getOptionalString("like");
    return viewerState;
}

ContentMode stringToContentMode(const QString& str)
{
    static const std::unordered_map<QString, ContentMode> mapping = {
        { "app.bsky.feed.defs#contentModeUnspecified", ContentMode::UNSPECIFIED },
        { "app.bsky.feed.defs#contentModeVideo", ContentMode::VIDEO }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    qDebug() << "Unknown content mode:" << str;
    return ContentMode::UNKNOWN;
}

QString contentModeToString(ContentMode mode, const QString& unknown)
{
    static const std::unordered_map<ContentMode, QString> mapping = {
        { ContentMode::UNSPECIFIED, "app.bsky.feed.defs#contentModeUnspecified" },
        { ContentMode::VIDEO, "app.bsky.feed.defs#contentModeVideo" }
    };

    const auto it = mapping.find(mode);
    if (it != mapping.end())
        return it->second;

    qDebug() << "Unknown content mode:" << (int)mode;
    return unknown;
}

QJsonObject GeneratorView::toJson() const
{
    QJsonObject json;
    json.insert("$type", "app.bsky.feed.defs#generatorView");
    json.insert("uri", mUri);
    json.insert("cid", mCid);
    json.insert("did", mDid);
    json.insert("creator", mCreator->toJson());
    json.insert("displayName", mDisplayName);
    XJsonObject::insertOptionalJsonValue(json, "description", mDescription);
    XJsonObject::insertOptionalArray<AppBskyRichtext::Facet>(json, "descriptionFacets", mDescriptionFacets);
    XJsonObject::insertOptionalJsonValue(json, "avatar", mAvatar);
    XJsonObject::insertOptionalJsonValue(json, "likeCount", mLikeCount, 0);
    XJsonObject::insertOptionalJsonValue(json, "acceptsInteractions", mAcceptsInteractions, false);
    XJsonObject::insertOptionalJsonObject<GeneratorViewerState>(json, "viewer", mViewer);

    if (mContentMode)
        json.insert("contentMode", contentModeToString(*mContentMode, mRawContentMode.value_or("app.bsky.feed.defs#contentModeUnspecified")));
    else
        json.remove("contentMode");

    json.insert("indexedAt", mIndexedAt.toString(Qt::ISODateWithMs));
    return json;
}

GeneratorView::SharedPtr GeneratorView::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<GeneratorView>();
    const XJsonObject xjson(json);
    view->mUri = xjson.getRequiredString("uri");
    view->mCid = xjson.getRequiredString("cid");
    view->mDid = xjson.getRequiredString("did");
    view->mCreator = xjson.getRequiredObject<AppBskyActor::ProfileView>("creator");
    view->mDisplayName = xjson.getRequiredString("displayName");
    view->mDescription = xjson.getOptionalString("description");
    view->mDescriptionFacets = xjson.getOptionalVector<AppBskyRichtext::Facet>("descriptionFacets");
    view->mAvatar = xjson.getOptionalString("avatar");
    view->mLikeCount = xjson.getOptionalInt("likeCount", 0);
    view->mAcceptsInteractions = xjson.getOptionalBool("acceptsInteractions", false);
    ComATProtoLabel::getLabels(view->mLabels, json);
    view->mViewer = xjson.getOptionalObject<GeneratorViewerState>("viewer");
    view->mRawContentMode = xjson.getOptionalString("contentMode");

    if (view->mRawContentMode)
        view->mContentMode = stringToContentMode(*view->mRawContentMode);

    view->mIndexedAt = xjson.getRequiredDateTime("indexedAt");
    return view;
}

GetFeedGeneratorOutput::SharedPtr GetFeedGeneratorOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetFeedGeneratorOutput>();
    const XJsonObject xjson(json);
    output->mView = xjson.getRequiredObject<GeneratorView>("view");
    output->mIsOnline = xjson.getRequiredBool("isOnline");
    output->mIsValid = xjson.getRequiredBool("isValid");
    return output;
}

GetFeedGeneratorsOutput::SharedPtr GetFeedGeneratorsOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetFeedGeneratorsOutput>();
    const XJsonObject xjson(json);
    output->mFeeds = xjson.getRequiredVector<GeneratorView>("feeds");
    return output;
}

GetActorFeedsOutput::SharedPtr GetActorFeedsOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetActorFeedsOutput>();
    const XJsonObject xjson(json);
    output->mFeeds = xjson.getRequiredVector<GeneratorView>("feeds");
    output->mCursor = xjson.getOptionalString("cursor");
    return output;
}

GetQuotesOutput::SharedPtr GetQuotesOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetQuotesOutput>();
    const XJsonObject xjson(json);
    output->mUri = xjson.getRequiredString("uri");
    output->mCid = xjson.getOptionalString("cid");
    output->mCursor = xjson.getOptionalString("cursor");
    output->mPosts = xjson.getRequiredVector<PostView>("posts");
    return output;
}

QString Interaction::eventTypeToString(EventType eventType)
{
    static const std::unordered_map<EventType, QString> mapping = {
        { EventType::RequestLess, "requestLess" },
        { EventType::RequestMore, "requestMore" }
    };

    const auto it = mapping.find(eventType);
    Q_ASSERT(it != mapping.end());

    if (it == mapping.end())
    {
        qWarning() << "Unknown allow event type:" << int(eventType);
        return {};
    }

    return it->second;
}

QJsonObject Interaction::toJson() const
{
    QJsonObject json;
    XJsonObject::insertOptionalJsonValue(json, "item", mItem);

    if (mEvent)
        json.insert("event", eventTypeToString(*mEvent));

    XJsonObject::insertOptionalJsonValue(json, "feedContext", mFeedContext);
    return json;
}

}
