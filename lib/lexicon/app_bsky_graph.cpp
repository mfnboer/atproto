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

}
