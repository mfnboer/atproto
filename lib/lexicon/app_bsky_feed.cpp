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
    XJsonObject root = QJsonObject(json); // TODO copy?
    post->mText = root.getRequiredString("text");
    post->mCreatedAt = root.getRequiredDateTime("createdAt");
    return post;
}

PostView::Ptr PostView::fromJson(const QJsonObject& json)
{
    auto postView = std::make_unique<PostView>();
    XJsonObject root = QJsonObject(json); // TODO copy?
    postView->mUri = root.getRequiredString("uri");
    postView->mCid = root.getRequiredString("cid");
    postView->mAuthor = AppBskyActor::ProfileViewBasic::fromJson(json["author"].toObject());
    const XJsonObject record = root.getRequiredObject("record");
    const QString recordType = record.getRequiredString("$type");

    if (recordType != "app.bsky.feed.post")
        throw InvalidJsonException(QString("Unsupported record $type: %1").arg(recordType));

    postView->mRecord = Record::Post::fromJson(record.getObject());
    postView->mReplyCount = root.getOptionalInt("replyCount");
    postView->mRepostCount = root.getOptionalInt("repostCount");
    postView->mLikeCount = root.getOptionalInt("likeCount");
    postView->mIndexedAt = root.getRequiredDateTime("indexedAt");
    return postView;
}

FeedViewPost::Ptr FeedViewPost::fromJson(const QJsonObject& json)
{
    auto feedViewPost = std::make_unique<FeedViewPost>();
    XJsonObject root = QJsonObject(json); // TODO copy?
    const QJsonObject post = root.getRequiredObject("post");
    feedViewPost->mPost = PostView::fromJson(post);
    return feedViewPost;
}

AuthorFeed::Ptr AuthorFeed::fromJson(const QJsonDocument& json)
{
    auto authorFeed = std::make_unique<AuthorFeed>();
    XJsonObject root(json.object());
    authorFeed->mCursor = root.getOptionalString("cursor");
    const QJsonArray& feeds = root.getRequiredArray("feed");
    authorFeed->mFeed.reserve(feeds.size());

    for (const auto& feed : feeds)
    {
        if (!feed.isObject())
            throw InvalidJsonException("Invalid feed element");

        auto feedViewPost = FeedViewPost::fromJson(feed.toObject());
        authorFeed->mFeed.push_back(std::move(feedViewPost));
    }

    return authorFeed;
}

}
