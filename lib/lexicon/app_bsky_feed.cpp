// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_feed.h"
#include "app_bsky_actor.h"
#include "../xjson.h"
#include <QJsonArray>
#include <unordered_map>

namespace ATProto::AppBskyFeed {

ViewerState::SharedPtr ViewerState::fromJson(const QJsonObject& json)
{
    auto viewerState = std::make_shared<ViewerState>();
    XJsonObject xjson(json);
    viewerState->mRepost = xjson.getOptionalString("repost");
    viewerState->mLike = xjson.getOptionalString("like");
    viewerState->mThreadMuted = xjson.getOptionalBool("threadMuted", false);
    viewerState->mReplyDisabled = xjson.getOptionalBool("replyDisabled", false);
    return viewerState;
}

QJsonObject ThreadgateListRule::toJson() const
{
    QJsonObject json;
    json.insert("$type", "app.bsky.feed.threadgate#listRule");
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

QJsonObject Threadgate::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("post", mPost);

    QJsonArray allowArray;

    if (mAllowMention)
    {
        QJsonObject rule;
        rule.insert("$type", "app.bsky.feed.threadgate#mentionRule");
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

    json.insert("allow", allowArray);
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
        for (const auto& allowElem : *allowArray)
        {
            if (!allowElem.isObject())
            {
                qWarning() << "PROTO ERROR invalid threadgate allow element: not an object";
                qInfo() << json;
                throw InvalidJsonException("PROTO ERROR invalid threadgate element: allow");
            }

            auto allowJson = allowElem.toObject();
            XJsonObject allowXJson(allowJson);
            QString type = allowXJson.getRequiredString("$type");

            if (type == "app.bsky.feed.threadgate#mentionRule")
            {
                threadgate->mAllowMention = true;
            }
            else if (type == "app.bsky.feed.threadgate#followingRule")
            {
                threadgate->mAllowFollowing = true;
            }
            else if (type == "app.bsky.feed.threadgate#listRule")
            {
                auto listRule = ThreadgateListRule::fromJson(allowJson);
                threadgate->mAllowList.push_back(std::move(listRule));
            }
            else
            {
                qWarning() << "Unknown threadgate rule type:" << type;
            }
        }
    }

    threadgate->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return threadgate;
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
    QJsonObject json;
    json.insert("$type", "app.bsky.feed.post");
    json.insert("text", mText);
    json.insert("facets", XJsonObject::toJsonArray<AppBskyRichtext::Facet>(mFacets));
    XJsonObject::insertOptionalJsonObject<PostReplyRef>(json, "reply", mReply);
    XJsonObject::insertOptionalJsonObject<AppBskyEmbed::Embed>(json, "embed", mEmbed);
    XJsonObject::insertOptionalJsonObject<ComATProtoLabel::SelfLabels>(json, "labels", mLabels);

    if (!mLanguages.empty())
        json.insert("langs", XJsonObject::toJsonArray(mLanguages));

    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));
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
    return post;
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
    postView->mIndexedAt = xjson.getRequiredDateTime("indexedAt");
    postView->mViewer = xjson.getOptionalObject<ViewerState>("viewer");
    ComATProtoLabel::getLabels(postView->mLabels, json);
    postView->mThreadgate = xjson.getOptionalObject<ThreadgateView>("threadgate");
    return postView;
}

void getPostViewList(PostViewList& list, const QJsonObject& json)
{
    XJsonObject xjson(json);
    list = xjson.getRequiredVector<PostView>("posts");
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

ReplyRef::SharedPtr ReplyRef::fromJson(const QJsonObject& json)
{
    auto replyRef = std::make_shared<ReplyRef>();
    XJsonObject xjson(json);
    replyRef->mRoot = xjson.getRequiredObject<ReplyElement>("root");
    replyRef->mParent = xjson.getRequiredObject<ReplyElement>("parent");
    replyRef->mGrandparentAuthor = xjson.getOptionalObject<AppBskyActor::ProfileViewBasic>("grandparentAuthor");
    return replyRef;
}

ReasonRepost::SharedPtr ReasonRepost::fromJson(const QJsonObject& json)
{
    auto reason = std::make_shared<ReasonRepost>();
    XJsonObject xjson(json);
    reason->mBy = xjson.getRequiredObject<AppBskyActor::ProfileViewBasic>("by");
    reason->mIndexedAt = xjson.getRequiredDateTime("indexedAt");
    return reason;
}

FeedViewPost::SharedPtr FeedViewPost::fromJson(const QJsonObject& json)
{
    auto feedViewPost = std::make_shared<FeedViewPost>();
    XJsonObject xjson(json);
    feedViewPost->mPost = xjson.getRequiredObject<PostView>("post");
    feedViewPost->mReply = xjson.getOptionalObject<ReplyRef>("reply");
    feedViewPost->mReason = xjson.getOptionalObject<ReasonRepost>("reason");
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

OutputFeed::SharedPtr OutputFeed::fromJson(const QJsonDocument& json)
{
    auto outputFeed = std::make_shared<OutputFeed>();
    const auto jsonObj = json.object();
    XJsonObject xjson(jsonObj);
    outputFeed->mCursor = xjson.getOptionalString("cursor");
    getFeed(outputFeed->mFeed, jsonObj);
    return outputFeed;
}

NotFoundPost::SharedPtr NotFoundPost::fromJson(const QJsonObject& json)
{
    auto notFound = std::make_shared<NotFoundPost>();
    const XJsonObject xjson(json);
    notFound->mUri = xjson.getRequiredString("uri");
    return notFound;
}

BlockedAuthor::SharedPtr BlockedAuthor::fromJson(const QJsonObject& json)
{
    auto blockedAuthor = std::make_shared<BlockedAuthor>();
    const XJsonObject xjson(json);
    blockedAuthor->mDid = xjson.getRequiredString("did");
    return blockedAuthor;
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
        { "app.bsky.feed.defs#postView", PostElementType::POST_VIEW },
        { "app.bsky.feed.defs#threadViewPost", PostElementType::THREAD_VIEW_POST },
        { "app.bsky.feed.defs#notFoundPost", PostElementType::NOT_FOUND_POST },
        { "app.bsky.feed.defs#blockedPost", PostElementType::BLOCKED_POST }
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
    return postThread;
}

QJsonObject Like::toJson() const
{
    QJsonObject json;
    json.insert("$type", "app.bsky.feed.like");
    json.insert("subject", mSubject->toJson());
    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));
    return json;
}

Like::SharedPtr Like::fromJson(const QJsonObject& json)
{
    auto like = std::make_shared<Like>();
    const XJsonObject xjson(json);
    like->mSubject = xjson.getRequiredObject<ComATProtoRepo::StrongRef>("subject");
    like->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return like;
}

QJsonObject Repost::toJson() const
{
    QJsonObject json;
    json.insert("$type", "app.bsky.feed.repost");
    json.insert("subject", mSubject->toJson());
    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));
    return json;
}

Repost::SharedPtr Repost::fromJson(const QJsonObject& json)
{
    auto repost = std::make_shared<Repost>();
    const XJsonObject xjson(json);
    repost->mSubject = xjson.getRequiredObject<ComATProtoRepo::StrongRef>("subject");
    repost->mCreatedAt = xjson.getRequiredDateTime("createdAt");
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
    getPostViewList(output->mPosts, json);
    return output;
}

GeneratorViewerState::SharedPtr GeneratorViewerState::fromJson(const QJsonObject& json)
{
    auto viewerState = std::make_shared<GeneratorViewerState>();
    const XJsonObject xjson(json);
    viewerState->mLike = xjson.getOptionalString("like");
    return viewerState;
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
    XJsonObject::insertOptionalJsonValue(json, "avatar", mAvatar);
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
