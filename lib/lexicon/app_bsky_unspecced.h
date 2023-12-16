// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_feed.h"
#include <QJsonObject>

namespace ATProto::AppBskyUnspecced {

// app.bsky.unspecced.getPopularFeedGenerators#output
struct GetPopularFeedGeneratorsOutput
{
    AppBskyFeed::GeneratorViewList mFeeds;
    std::optional<QString> mCursor;

    using Ptr = std::unique_ptr<GetPopularFeedGeneratorsOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

}
