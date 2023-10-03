// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_graph.h"
#include "../xjson.h"

namespace ATProto::AppBskyGraph {

GetFollowsOutput::Ptr GetFollowsOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto follows = std::make_unique<GetFollowsOutput>();
    const auto subjectJson = xjson.getRequiredObject("subject");
    follows->mSubject = AppBskyActor::ProfileView::fromJson(subjectJson);
    const auto followsJsonArray = xjson.getRequiredArray("follows");

    for (const auto& profileJson : followsJsonArray)
    {
        if (!profileJson.isObject())
            throw InvalidJsonException("Invalid follows profile");

        auto profile = AppBskyActor::ProfileView::fromJson(profileJson.toObject());
        follows->mFollows.push_back(std::move(profile));
    }

    follows->mCursor = xjson.getOptionalString("cursor");
    return follows;
}

GetFollowersOutput::Ptr GetFollowersOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto followers = std::make_unique<GetFollowersOutput>();
    const auto subjectJson = xjson.getRequiredObject("subject");
    followers->mSubject = AppBskyActor::ProfileView::fromJson(subjectJson);
    const auto followersJsonArray = xjson.getRequiredArray("followers");

    for (const auto& profileJson : followersJsonArray)
    {
        if (!profileJson.isObject())
            throw InvalidJsonException("Invalid followers profile");

        auto profile = AppBskyActor::ProfileView::fromJson(profileJson.toObject());
        followers->mFollowers.push_back(std::move(profile));
    }

    followers->mCursor = xjson.getOptionalString("cursor");
    return followers;
}

QJsonObject Follow::toJson() const
{
    QJsonObject json;
    json.insert("$type", "app.bsky.graph.follow");
    json.insert("subject", mSubject);
    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));
    return json;
}

Follow::Ptr Follow::fromJson(const QJsonObject& json)
{
    auto follow = std::make_unique<Follow>();
    XJsonObject xjson(json);
    follow->mSubject = xjson.getRequiredString("subject");
    follow->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return follow;
}

QJsonObject Block::toJson() const
{
    QJsonObject json;
    json.insert("$type", "app.bsky.graph.block");
    json.insert("subject", mSubject);
    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));
    return json;
}

Block::Ptr Block::fromJson(const QJsonObject& json)
{
    auto block = std::make_unique<Block>();
    XJsonObject xjson(json);
    block->mSubject = xjson.getRequiredString("subject");
    block->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return block;
}

}
