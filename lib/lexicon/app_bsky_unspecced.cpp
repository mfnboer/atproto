// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_unspecced.h"
#include "../xjson.h"

namespace ATProto::AppBskyUnspecced {

GetPopularFeedGeneratorsOutput::SharedPtr GetPopularFeedGeneratorsOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetPopularFeedGeneratorsOutput>();
    const XJsonObject xjson(json);
    output->mFeeds = xjson.getRequiredVector<AppBskyFeed::GeneratorView>("feeds");
    output->mCursor = xjson.getOptionalString("cursor");
    return output;
}

TrendingTopic::SharedPtr TrendingTopic::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<TrendingTopic>();
    const XJsonObject xjson(json);
    output->mTopic = xjson.getRequiredString("topic");
    output->mDisplayName = xjson.getOptionalString("displayName");
    output->mDescription = xjson.getOptionalString("description");
    output->mLink = xjson.getRequiredString("link");
    return output;
}

GetTrendingTopicsOutput::SharedPtr GetTrendingTopicsOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetTrendingTopicsOutput>();
    const XJsonObject xjson(json);
    output->mTopics = xjson.getRequiredVector<TrendingTopic>("topics");
    output->mSuggested = xjson.getRequiredVector<TrendingTopic>("suggested");
    return output;
}

}
