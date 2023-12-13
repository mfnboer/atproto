// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_graph.h"
#include "../xjson.h"

namespace ATProto::AppBskyGraph {

GetFollowsOutput::Ptr GetFollowsOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto follows = std::make_unique<GetFollowsOutput>();
    follows->mSubject = xjson.getRequiredObject<AppBskyActor::ProfileView>("subject");
    AppBskyActor::getProfileViewList(follows->mFollows, json, "follows");
    follows->mCursor = xjson.getOptionalString("cursor");
    return follows;
}

GetFollowersOutput::Ptr GetFollowersOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto followers = std::make_unique<GetFollowersOutput>();
    followers->mSubject = xjson.getRequiredObject<AppBskyActor::ProfileView>("subject");
    AppBskyActor::getProfileViewList(followers->mFollowers, json, "followers");
    followers->mCursor = xjson.getOptionalString("cursor");
    return followers;
}

GetBlocksOutput::Ptr GetBlocksOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto blocks = std::make_unique<GetBlocksOutput>();
    AppBskyActor::getProfileViewList(blocks->mBlocks, json, "blocks");
    blocks->mCursor = xjson.getOptionalString("cursor");
    return blocks;
}

GetMutesOutput::Ptr GetMutesOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto blocks = std::make_unique<GetMutesOutput>();
    AppBskyActor::getProfileViewList(blocks->mMutes, json, "mutes");
    blocks->mCursor = xjson.getOptionalString("cursor");
    return blocks;
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

ListViewerState::Ptr ListViewerState::fromJson(const QJsonObject& json)
{
    auto viewerState = std::make_unique<ListViewerState>();
    XJsonObject xjson(json);
    viewerState->mMuted = xjson.getOptionalBool("muted", false);
    viewerState->mBlocked = xjson.getOptionalString("blocked");
    return viewerState;
}

ListViewBasic::Ptr ListViewBasic::fromJson(const QJsonObject& json)
{
    auto listView = std::make_unique<ListViewBasic>();
    XJsonObject xjson(json);
    listView->mUri = xjson.getRequiredString("uri");
    listView->mCid = xjson.getRequiredString("cid");
    listView->mName = xjson.getRequiredString("name");
    listView->mPurpose = xjson.getRequiredString("purpose");
    listView->mAvatar = xjson.getOptionalString("avatar");
    listView->mViewer = xjson.getOptionalObject<ListViewerState>("viewer");
    listView->mIndexedAt = xjson.getOptionalDateTime("indexedAt");
    return listView;
}

}
