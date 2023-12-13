// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_feed.h"
#include "app_bsky_actor.h"
#include "../at_uri.h"
#include "../xjson.h"
#include <QJsonArray>
#include <unordered_map>

namespace ATProto::AppBskyFeed {

ViewerState::Ptr ViewerState::fromJson(const QJsonObject& json)
{
    auto viewerState = std::make_unique<ViewerState>();
    XJsonObject xjson(json);
    viewerState->mRepost = xjson.getOptionalString("repost");
    viewerState->mLike = xjson.getOptionalString("like");
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

ThreadgateListRule::Ptr ThreadgateListRule::fromJson(const QJsonObject& json)
{
    auto rule = std::make_unique<ThreadgateListRule>();
    XJsonObject xjson(json);
    rule->mList = xjson.getRequiredString("list");
    return rule;
}

QJsonObject Threadgate::toJson() const
{
    QJsonObject json;
    json.insert("$type", "app.bsky.feed.threadgate");
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

Threadgate::Ptr Threadgate::fromJson(const QJsonObject& json)
{
    auto threadgate = std::make_unique<Threadgate>();
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

ThreadgateView::Ptr ThreadgateView::fromJson(const QJsonObject& json)
{
    auto threadgateView = std::make_unique<ThreadgateView>();
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

PostReplyRef::Ptr PostReplyRef::fromJson(const QJsonObject& json)
{
    auto replyRef = std::make_unique<PostReplyRef>();
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
    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));

    if (mReply)
        json.insert("reply", mReply->toJson());

    if (mEmbed)
        json.insert("embed", mEmbed->toJson());

    if (!mFacets.empty())
    {
        QJsonArray jsonArray;
        for (const auto& facet : mFacets)
        {
            QJsonObject facetJson = facet->toJson();
            jsonArray.append(facetJson);
        }

        json.insert("facets", jsonArray);
    }

    return json;
}

Record::Post::Ptr Record::Post::fromJson(const QJsonObject& json)
{
    auto post = std::make_unique<Record::Post>();
    XJsonObject xjson(json);
    post->mText = xjson.getRequiredString("text");
    post->mFacets = xjson.getOptionalVector<AppBskyRichtext::Facet>("facets");
    post->mReply = xjson.getOptionalObject<PostReplyRef>("reply");
    post->mEmbed = xjson.getOptionalObject<AppBskyEmbed::Embed>("embed");
    post->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return post;
}

PostView::Ptr PostView::fromJson(const QJsonObject& json)
{
    auto postView = std::make_unique<PostView>();
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

ReplyElement::Ptr ReplyElement::fromJson(const QJsonObject& json)
{
    auto element = std::make_unique<ReplyElement>();
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

ReplyRef::Ptr ReplyRef::fromJson(const QJsonObject& json)
{
    auto replyRef = std::make_unique<ReplyRef>();
    XJsonObject xjson(json);
    replyRef->mRoot = xjson.getRequiredObject<ReplyElement>("root");
    replyRef->mParent = xjson.getRequiredObject<ReplyElement>("parent");
    return replyRef;
}

ReasonRepost::Ptr ReasonRepost::fromJson(const QJsonObject& json)
{
    auto reason = std::make_unique<ReasonRepost>();
    XJsonObject xjson(json);
    reason->mBy = xjson.getRequiredObject<AppBskyActor::ProfileViewBasic>("by");
    reason->mIndexedAt = xjson.getRequiredDateTime("indexedAt");
    return reason;
}

FeedViewPost::Ptr FeedViewPost::fromJson(const QJsonObject& json)
{
    auto feedViewPost = std::make_unique<FeedViewPost>();
    XJsonObject xjson(json);
    feedViewPost->mPost = xjson.getRequiredObject<PostView>("post");
    feedViewPost->mReply = xjson.getOptionalObject<ReplyRef>("reply");
    feedViewPost->mReason = xjson.getOptionalObject<ReasonRepost>("reason");
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

OutputFeed::Ptr OutputFeed::fromJson(const QJsonDocument& json)
{
    auto outputFeed = std::make_unique<OutputFeed>();
    const auto jsonObj = json.object();
    XJsonObject xjson(jsonObj);
    outputFeed->mCursor = xjson.getOptionalString("cursor");
    getFeed(outputFeed->mFeed, jsonObj);
    return outputFeed;
}

NotFoundPost::Ptr NotFoundPost::fromJson(const QJsonObject& json)
{
    auto notFound = std::make_unique<NotFoundPost>();
    const XJsonObject xjson(json);
    notFound->mUri = xjson.getRequiredString("uri");
    return notFound;
}

BlockedAuthor::Ptr BlockedAuthor::fromJson(const QJsonObject& json)
{
    auto blockedAuthor = std::make_unique<BlockedAuthor>();
    const XJsonObject xjson(json);
    blockedAuthor->mDid = xjson.getRequiredString("did");
    return blockedAuthor;
}

BlockedPost::Ptr BlockedPost::fromJson(const QJsonObject& json)
{
    auto blockedPost = std::make_unique<BlockedPost>();
    const XJsonObject xjson(json);
    blockedPost->mUri = xjson.getRequiredString("uri");
    blockedPost->mAuthor = xjson.getRequiredObject<BlockedAuthor>("author");
    return blockedPost;
}

ThreadViewPost::Ptr ThreadViewPost::fromJson(const QJsonObject& json)
{
    auto thread = std::make_unique<ThreadViewPost>();
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

ThreadElement::Ptr ThreadElement::fromJson(const QJsonObject& json)
{
    auto element = std::make_unique<ThreadElement>();
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

PostThread::Ptr PostThread::fromJson(const QJsonObject& json)
{
    auto postThread = std::make_unique<PostThread>();
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

Like::Ptr Like::fromJson(const QJsonObject& json)
{
    auto like = std::make_unique<Like>();
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

Repost::Ptr Repost::fromJson(const QJsonObject& json)
{
    auto repost = std::make_unique<Repost>();
    const XJsonObject xjson(json);
    repost->mSubject = xjson.getRequiredObject<ComATProtoRepo::StrongRef>("subject");
    repost->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return repost;
}

GetLikesLike::Ptr GetLikesLike::fromJson(const QJsonObject& json)
{
    auto like = std::make_unique<GetLikesLike>();
    const XJsonObject xjson(json);
    like->mIndexedAt = xjson.getRequiredDateTime("indexedAt");
    like->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    like->mActor = xjson.getRequiredObject<AppBskyActor::ProfileView>("actor");
    return like;
}

GetLikesOutput::Ptr GetLikesOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_unique<GetLikesOutput>();
    const XJsonObject xjson(json);
    output->mUri = xjson.getRequiredString("uri");
    output->mCid = xjson.getOptionalString("cid");
    output->mCursor = xjson.getOptionalString("cursor");
    output->mLikes = xjson.getRequiredVector<GetLikesLike>("likes");
    return output;
}

GetRepostedByOutput::Ptr GetRepostedByOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_unique<GetRepostedByOutput>();
    const XJsonObject xjson(json);
    output->mUri = xjson.getRequiredString("uri");
    output->mCid = xjson.getOptionalString("cid");
    output->mCursor = xjson.getOptionalString("cursor");
    output->mRepostedBy = xjson.getRequiredVector<AppBskyActor::ProfileView>("repostedBy");
    return output;
}

SearchPostsOutput::Ptr SearchPostsOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_unique<SearchPostsOutput>();
    const XJsonObject xjson(json);
    output->mCursor = xjson.getOptionalString("cursor");
    output->mHitsTotal = xjson.getOptionalInt("hitsTotal");
    getPostViewList(output->mPosts, json);
    return output;
}

LegacySearchPostsOutput::Ptr LegacySearchPostsOutput::fromJson(const QJsonArray& jsonArray)
{
    static const QRegularExpression RE_TID(R"(^app.bsky.feed.post/([a-zA-Z0-9\.-_~]+)$)");

    auto output = std::make_unique<LegacySearchPostsOutput>();
    output->mUris.reserve(jsonArray.size());

    for (const auto& postJsonEntry : jsonArray)
    {
        if (!postJsonEntry.isObject())
        {
            qWarning() << "Invalid post:" << jsonArray << postJsonEntry;
            throw InvalidJsonException("LegacySearchPostsOutput");
        }

        const QJsonObject postJson = postJsonEntry.toObject();
        const XJsonObject xjsonPost(postJson);
        const QString tid = xjsonPost.getRequiredString("tid");
        auto match = RE_TID.match(tid);

        if (!match.hasMatch())
        {
            qDebug() << "Unsupported tid:" << tid;
            continue;
        }

        const QString rKey = match.captured(1);
        const auto userJson = xjsonPost.getRequiredJsonObject("user");
        const XJsonObject xjsonUser(userJson);
        const QString did = xjsonUser.getRequiredString("did");

        ATUri atUri;
        atUri.setAuthority(did);
        atUri.setCollection("app.bsky.feed.post");
        atUri.setRKey(rKey);

        output->mUris.push_back(atUri.toString());
    }

    return output;
}

}
