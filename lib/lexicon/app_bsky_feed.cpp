// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_feed.h"
#include "app_bsky_actor.h"
#include "../xjson.h"
#include <QJsonArray>

namespace ATProto::AppBskyFeed {

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
    const QString type = record.getRequiredString("$type");
    postView->mRecordType = stringToRecordType(type);

    if (postView->mRecordType != RecordType::APP_BSKY_FEED_POST)
        throw InvalidJsonException(QString("Unsupported record type in app.bsky.feed.defs#postView: %1").arg(type));

    postView->mRecord = Record::Post::fromJson(record.getObject());

    const auto embedJson = xjson.getOptionalObject("embed");
    if (embedJson)
        postView->mEmbed = AppBskyEmbed::Embed::fromJson(*embedJson);

    postView->mReplyCount = xjson.getOptionalInt("replyCount");
    postView->mRepostCount = xjson.getOptionalInt("repostCount");
    postView->mLikeCount = xjson.getOptionalInt("likeCount");
    postView->mIndexedAt = xjson.getRequiredDateTime("indexedAt");
    return postView;
}

ReplyRef::Ptr ReplyRef::fromJson(const QJsonObject& json)
{
    auto replyRef = std::make_unique<ReplyRef>();
    XJsonObject xjson(json);
    const auto rootJson = xjson.getRequiredObject("root");
    replyRef->mRoot = PostView::fromJson(rootJson);
    const auto parentJson = xjson.getRequiredObject("parent");
    replyRef->mParent = PostView::fromJson(parentJson);
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

AuthorFeed::Ptr AuthorFeed::fromJson(const QJsonDocument& json)
{
    auto authorFeed = std::make_unique<AuthorFeed>();
    const auto jsonObj = json.object();
    XJsonObject xjson(jsonObj);
    authorFeed->mCursor = xjson.getOptionalString("cursor");
    const QJsonArray& feeds = xjson.getRequiredArray("feed");
    authorFeed->mFeed.reserve(feeds.size());

    for (const auto& feed : feeds)
    {
        if (!feed.isObject())
            throw InvalidJsonException("Invalid feed");

        auto feedViewPost = FeedViewPost::fromJson(feed.toObject());
        authorFeed->mFeed.push_back(std::move(feedViewPost));
    }

    return authorFeed;
}

}
