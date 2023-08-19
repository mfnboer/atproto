// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_feed.h"
#include "../xjson.h"
#include <QJsonArray>

namespace ATProto::AppBskyFeed {

AuthorFeed::Ptr AuthorFeed::fromJson(const QJsonDocument& json)
{
    XJsonObject root(json.object());
    auto authorFeed = std::make_unique<AuthorFeed>();
    authorFeed->mCursor = root.getOptionalString("cursor");

    const auto& feedArray = json["feed"];
    if (!feedArray.isArray())
        throw InvalidJsonException("feed array missing");

    const QJsonArray& feeds = feedArray.toArray();
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
