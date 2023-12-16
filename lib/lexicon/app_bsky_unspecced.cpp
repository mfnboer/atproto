// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_unspecced.h"
#include "../xjson.h"

namespace ATProto::AppBskyUnspecced {

GetPopularFeedGeneratorsOutput::Ptr GetPopularFeedGeneratorsOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_unique<GetPopularFeedGeneratorsOutput>();
    const XJsonObject xjson(json);
    output->mFeeds = xjson.getRequiredVector<AppBskyFeed::GeneratorView>("feeds");
    output->mCursor = xjson.getOptionalString("cursor");
    return output;
}

}
