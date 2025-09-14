// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_embed.h"
#include <QJsonObject>

namespace ATProto::AppBskyUnspecced {

// app.bsky.unspecced.getPopularFeedGenerators#output
struct GetPopularFeedGeneratorsOutput
{
    AppBskyFeed::GeneratorViewList mFeeds;
    std::optional<QString> mCursor;

    using SharedPtr = std::shared_ptr<GetPopularFeedGeneratorsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.unspecced.defs#trendingTopic
struct TrendingTopic
{
    QString mTopic;
    std::optional<QString> mDisplayName;
    std::optional<QString> mDescription;
    QString mLink;

    using SharedPtr = std::shared_ptr<TrendingTopic>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.unspecced.getTrendingTopics#output
struct GetTrendingTopicsOutput
{
    TrendingTopic::List mTopics;
    TrendingTopic::List mSuggested;

    using SharedPtr = std::shared_ptr<GetTrendingTopicsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

enum class TrendStatus
{
    HOT,
    UNKNOWN
};

TrendStatus stringToTrendStatus(const QString& str);

// app.bsky.unspecced.defs#trendView
struct TrendView
{
    QString mTopic;
    QString mDisplayName;
    QString mLink;
    QDateTime mStartedAt;
    std::optional<QString> mRawStatus;
    TrendStatus mStatus = TrendStatus::UNKNOWN;
    std::optional<QString> mCategory;
    int mPostCount = 0;
    AppBskyActor::ProfileViewBasicList mActors;

    using SharedPtr = std::shared_ptr<TrendView>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.unspecced.getTrends#output
struct GetTrendsOutput
{
    TrendView::List mTrends;

    using SharedPtr = std::shared_ptr<GetTrendsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct GetSuggestedStarterPacksOutput
{
    AppBskyGraph::StarterPackView::List mStarterPacks;

    using SharedPtr = std::shared_ptr<GetSuggestedStarterPacksOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}
