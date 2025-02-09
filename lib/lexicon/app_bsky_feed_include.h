// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>

// Extra header to break cyclic dependencies

namespace ATProto::AppBskyFeed {

// app.bsky.feed.postgate#disableRule
struct PostgateDisableRule
{
    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<PostgateDisableRule>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.feed.postgate#disableRule";
};

struct PostgateEmbeddingRules {
    using RuleType = std::variant<PostgateDisableRule::SharedPtr>;

    static void insertDisableEmbedding(QJsonObject& json, const QString& field, bool disableEmbedding);
    static bool getDisableEmbedding(const QJsonObject& json, const QString& field);
};

// app.bsky.feed.threadgate#listRule
struct ThreadgateListRule
{
    QString mList; // at-uri

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ThreadgateListRule>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.feed.threadgate#listRule";
};

struct ThreadgateRules
{
    bool mAllowNobody = false;
    bool mAllowMention = false;
    bool mAllowFollower = false;
    bool mAllowFollowing = false;
    std::vector<ThreadgateListRule::SharedPtr> mAllowList;

    QJsonArray toJson() const;

    using SharedPtr = std::shared_ptr<ThreadgateRules>;
    static SharedPtr fromJson(const QJsonArray& allowArray);
};

}
