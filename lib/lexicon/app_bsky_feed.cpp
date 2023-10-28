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
    return viewerState;
}

QJsonObject  PostReplyRef::toJson() const
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
    const auto rootJson = xjson.getRequiredObject("root");
    replyRef->mRoot = ComATProtoRepo::StrongRef::fromJson(rootJson);
    const auto parentJson = xjson.getRequiredObject("parent");
    replyRef->mParent = ComATProtoRepo::StrongRef::fromJson(parentJson);
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

    const std::optional<QJsonArray> facets = xjson.getOptionalArray("facets");
    if (facets)
    {
        for (const auto& facet : *facets)
        {
            if (!facet.isObject())
                throw InvalidJsonException("Invalid facet");

            auto f = AppBskyRichtext::Facet::fromJson(facet.toObject());
            post->mFacets.push_back(std::move(f));
        }
    }

    const auto replyJson = xjson.getOptionalObject("reply");
    if (replyJson)
        post->mReply = PostReplyRef::fromJson(*replyJson);

    const auto embedJson = xjson.getOptionalObject("embed");
    if (embedJson)
        post->mEmbed = AppBskyEmbed::Embed::fromJson(*embedJson);

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
    const auto recordObj = xjson.getRequiredObject("record");
    const XJsonObject record(recordObj);
    postView->mRawRecordType = record.getRequiredString("$type");
    postView->mRecordType = stringToRecordType(postView->mRawRecordType);

    if (postView->mRecordType == RecordType::APP_BSKY_FEED_POST)
        postView->mRecord = Record::Post::fromJson(record.getObject());
    else
        qWarning() << QString("Unsupported record type in app.bsky.feed.defs#postView: %1").arg(postView->mRawRecordType);

    const auto embedJson = xjson.getOptionalObject("embed");
    if (embedJson)
        postView->mEmbed = AppBskyEmbed::EmbedView::fromJson(*embedJson);

    postView->mReplyCount = xjson.getOptionalInt("replyCount", 0);
    postView->mRepostCount = xjson.getOptionalInt("repostCount", 0);
    postView->mLikeCount = xjson.getOptionalInt("likeCount", 0);
    postView->mIndexedAt = xjson.getRequiredDateTime("indexedAt");

    const auto viewerJson = xjson.getOptionalObject("viewer");
    if (viewerJson)
        postView->mViewer = ViewerState::fromJson(*viewerJson);

    ComATProtoLabel::getLabels(postView->mLabels, json);
    return postView;
}

void getPostViewList(PostViewList& list, const QJsonObject& json)
{
    XJsonObject xjson(json);

    const QJsonArray& listArray = xjson.getRequiredArray("posts");
    list.reserve(listArray.size());

    for (const auto& postViewJson : listArray)
    {
        if (!postViewJson.isObject())
        {
            qWarning() << "PROTO ERROR invalid list element: not an object";
            qInfo() << json;
            throw InvalidJsonException("PROTO ERROR invalid PostViewList element: not an object");
        }

        auto postView = PostView::fromJson(postViewJson.toObject());
        list.push_back(std::move(postView));
    }
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
    const auto rootJson = xjson.getRequiredObject("root");
    replyRef->mRoot = ReplyElement::fromJson(rootJson);
    const auto parentJson = xjson.getRequiredObject("parent");
    replyRef->mParent = ReplyElement::fromJson(parentJson);
    return replyRef;
}

ReasonRepost::Ptr ReasonRepost::fromJson(const QJsonObject& json)
{
    auto reason = std::make_unique<ReasonRepost>();
    XJsonObject xjson(json);
    const auto byJson = xjson.getRequiredObject("by");
    reason->mBy = AppBskyActor::ProfileViewBasic::fromJson(byJson);
    reason->mIndexedAt = xjson.getRequiredDateTime("indexedAt");
    return reason;
}

FeedViewPost::Ptr FeedViewPost::fromJson(const QJsonObject& json)
{
    auto feedViewPost = std::make_unique<FeedViewPost>();
    XJsonObject xjson(json);
    const auto postJson = xjson.getRequiredObject("post");
    feedViewPost->mPost = PostView::fromJson(postJson);

    const auto replyJson = xjson.getOptionalObject("reply");
    if (replyJson)
        feedViewPost->mReply = ReplyRef::fromJson(*replyJson);

    const auto reasonJson = xjson.getOptionalObject("reason");
    if (reasonJson)
        feedViewPost->mReason = ReasonRepost::fromJson(*reasonJson);

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
    const auto authorJson = xjson.getRequiredObject("author");
    blockedPost->mAuthor = BlockedAuthor::fromJson(authorJson);
    return blockedPost;
}

ThreadViewPost::Ptr ThreadViewPost::fromJson(const QJsonObject& json)
{
    auto thread = std::make_unique<ThreadViewPost>();
    const XJsonObject xjson(json);
    const auto postJson = xjson.getRequiredObject("post");
    thread->mPost = PostView::fromJson(postJson);
    const auto parentJson = xjson.getOptionalObject("parent");

    if (parentJson)
        thread->mParent = ThreadElement::fromJson(*parentJson);

    const auto repliesJson = xjson.getOptionalArray("replies");

    if (repliesJson)
    {
        for (const auto& replyJson : *repliesJson)
        {
            if (!replyJson.isObject())
            {
                qWarning() << "Invalid feed:" << json;
                throw InvalidJsonException("threadViewPost element");
            }

            auto reply = ThreadElement::fromJson(replyJson.toObject());

            if (reply)
                thread->mReplies.push_back(std::move(reply));
        }
    }

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
    const auto threadJson = xjson.getRequiredObject("thread");
    postThread->mThread = ThreadElement::fromJson(threadJson);
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
    const auto subjectJson = xjson.getRequiredObject("subject");
    like->mSubject = ComATProtoRepo::StrongRef::fromJson(subjectJson);
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
    const auto subjectJson = xjson.getRequiredObject("subject");
    repost->mSubject = ComATProtoRepo::StrongRef::fromJson(subjectJson);
    repost->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return repost;
}

GetLikesLike::Ptr GetLikesLike::fromJson(const QJsonObject& json)
{
    auto like = std::make_unique<GetLikesLike>();
    const XJsonObject xjson(json);
    like->mIndexedAt = xjson.getRequiredDateTime("indexedAt");
    like->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    const auto actorJson = xjson.getRequiredObject("actor");
    like->mActor = AppBskyActor::ProfileView::fromJson(actorJson);
    return like;
}

GetLikesOutput::Ptr GetLikesOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_unique<GetLikesOutput>();
    const XJsonObject xjson(json);
    output->mUri = xjson.getRequiredString("uri");
    output->mCid = xjson.getOptionalString("cid");
    output->mCursor = xjson.getOptionalString("cursor");

    const auto likesJson = xjson.getRequiredArray("likes");

    for (const auto& likeJson : likesJson)
    {
        if (!likeJson.isObject())
        {
            qWarning() << "Invalid like:" << json;
            throw InvalidJsonException("likes output");
        }

        auto like = GetLikesLike::fromJson(likeJson.toObject());
        output->mLikes.push_back(std::move(like));
    }

    return output;
}

GetRepostedByOutput::Ptr GetRepostedByOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_unique<GetRepostedByOutput>();
    const XJsonObject xjson(json);
    output->mUri = xjson.getRequiredString("uri");
    output->mCid = xjson.getOptionalString("cid");
    output->mCursor = xjson.getOptionalString("cursor");

    const auto repostedByJson = xjson.getRequiredArray("repostedBy");

    for (const auto& profileJson : repostedByJson)
    {
        if (!profileJson.isObject())
        {
            qWarning() << "Invalid repostedBy:" << json;
            throw InvalidJsonException("repostedBy output");
        }

        auto profile = AppBskyActor::ProfileView::fromJson(profileJson.toObject());
        output->mRepostedBy.push_back(std::move(profile));
    }

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
        const auto userJson = xjsonPost.getRequiredObject("user");
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
