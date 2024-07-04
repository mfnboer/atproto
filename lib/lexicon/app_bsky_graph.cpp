// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_graph.h"
#include "app_bsky_embed.h"
#include "../xjson.h"
#include <unordered_map>

namespace ATProto::AppBskyGraph {

GetFollowsOutput::Ptr GetFollowsOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto follows = std::make_unique<GetFollowsOutput>();
    follows->mSubject = xjson.getRequiredObject<AppBskyActor::ProfileView>("subject");
    follows->mFollows = xjson.getRequiredVector<AppBskyActor::ProfileView>("follows");
    follows->mCursor = xjson.getOptionalString("cursor");
    return follows;
}

GetFollowersOutput::Ptr GetFollowersOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto followers = std::make_unique<GetFollowersOutput>();
    followers->mSubject = xjson.getRequiredObject<AppBskyActor::ProfileView>("subject");
    followers->mFollowers = xjson.getRequiredVector<AppBskyActor::ProfileView>("followers");
    followers->mCursor = xjson.getOptionalString("cursor");
    return followers;
}

GetBlocksOutput::Ptr GetBlocksOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto blocks = std::make_unique<GetBlocksOutput>();
    blocks->mBlocks = xjson.getRequiredVector<AppBskyActor::ProfileView>("blocks");
    blocks->mCursor = xjson.getOptionalString("cursor");
    return blocks;
}

GetMutesOutput::Ptr GetMutesOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto blocks = std::make_unique<GetMutesOutput>();
    blocks->mMutes = xjson.getRequiredVector<AppBskyActor::ProfileView>("mutes");
    blocks->mCursor = xjson.getOptionalString("cursor");
    return blocks;
}

QJsonObject Follow::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", "app.bsky.graph.follow");
    json.insert("subject", mSubject);
    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));
    return json;
}

Follow::Ptr Follow::fromJson(const QJsonObject& json)
{
    auto follow = std::make_unique<Follow>();
    XJsonObject xjson(json);
    follow->mJson = json;
    follow->mSubject = xjson.getRequiredString("subject");
    follow->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return follow;
}

QJsonObject Block::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", "app.bsky.graph.block");
    json.insert("subject", mSubject);
    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));
    return json;
}

Block::Ptr Block::fromJson(const QJsonObject& json)
{
    auto block = std::make_unique<Block>();
    XJsonObject xjson(json);
    block->mJson = json;
    block->mSubject = xjson.getRequiredString("subject");
    block->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return block;
}

ListPurpose stringToListPurpose(const QString& str)
{
    static const std::unordered_map<QString, ListPurpose> mapping = {
        { "app.bsky.graph.defs#modlist", ListPurpose::MOD_LIST },
        { "app.bsky.graph.defs#curatelist", ListPurpose::CURATE_LIST },
        { "app.bsky.graph.defs#referencelist", ListPurpose::REFERENCE_LIST }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    return ListPurpose::UNKNOWN;
}

QString listPurposeToString(ListPurpose purpose)
{
    static const std::unordered_map<ListPurpose, QString> mapping = {
        { ListPurpose::MOD_LIST, "app.bsky.graph.defs#modlist" },
        { ListPurpose::CURATE_LIST, "app.bsky.graph.defs#curatelist" },
        { ListPurpose::REFERENCE_LIST, "app.bsky.graph.defs#referencelist" }
    };

    const auto it = mapping.find(purpose);
    if (it != mapping.end())
        return it->second;

    return {};
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
    listView->mRawPurpose = xjson.getRequiredString("purpose");
    listView->mPurpose = stringToListPurpose(listView->mRawPurpose);
    listView->mAvatar = xjson.getOptionalString("avatar");
    ComATProtoLabel::getLabels(listView->mLabels, json);
    listView->mViewer = xjson.getOptionalObject<ListViewerState>("viewer");
    listView->mIndexedAt = xjson.getOptionalDateTime("indexedAt");
    return listView;
}

QJsonObject ListView::toJson() const
{
    QJsonObject json;
    json.insert("$type", "app.bsky.graph.defs#listView");
    json.insert("uri", mUri);
    json.insert("cid", mCid);
    json.insert("creator", mCreator->toJson());
    json.insert("name", mName);
    json.insert("purpose", listPurposeToString(mPurpose));
    XJsonObject::insertOptionalJsonValue(json, "description", mDescription);
    XJsonObject::insertOptionalJsonValue(json, "avatar", mAvatar);
    return json;
}

ListView::Ptr ListView::fromJson(const QJsonObject& json)
{
    auto listView = std::make_unique<ListView>();
    XJsonObject xjson(json);
    listView->mUri = xjson.getRequiredString("uri");
    listView->mCid = xjson.getRequiredString("cid");
    listView->mCreator = xjson.getRequiredObject<AppBskyActor::ProfileView>("creator");
    listView->mName = xjson.getRequiredString("name");
    listView->mRawPurpose = xjson.getRequiredString("purpose");
    listView->mPurpose = stringToListPurpose(listView->mRawPurpose);
    listView->mDescription = xjson.getOptionalString("description");
    listView->mDescriptionFacets = xjson.getOptionalVector<AppBskyRichtext::Facet>("descriptionFacets");
    listView->mAvatar = xjson.getOptionalString("avatar");
    ComATProtoLabel::getLabels(listView->mLabels, json);
    listView->mViewer = xjson.getOptionalObject<ListViewerState>("viewer");
    listView->mIndexedAt = xjson.getOptionalDateTime("indexedAt");
    return listView;
}

ListItemView::Ptr ListItemView::fromJson(const QJsonObject& json)
{
    auto listItemView = std::make_unique<ListItemView>();
    XJsonObject xjson(json);
    listItemView->mUri = xjson.getRequiredString("uri");
    listItemView->mSubject = xjson.getRequiredObject<AppBskyActor::ProfileView>("subject");
    return listItemView;
}

QJsonObject List::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", "app.bsky.graph.list");

    if (mPurpose != ListPurpose::UNKNOWN)
        json.insert("purpose", listPurposeToString(mPurpose));
    else
        json.insert("purpose", mRawPurpose);

    json.insert("name", mName);
    XJsonObject::insertOptionalJsonValue(json, "description", mDescription);
    json.insert("descriptionFacets", XJsonObject::toJsonArray<AppBskyRichtext::Facet>(mDescriptionFacets));
    XJsonObject::insertOptionalJsonObject<Blob>(json, "avatar", mAvatar);
    XJsonObject::insertOptionalJsonObject<ComATProtoLabel::SelfLabels>(json, "labels", mLabels);
    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));
    return json;
}

List::Ptr List::fromJson(const QJsonObject& json)
{
    auto list = std::make_unique<List>();
    XJsonObject xjson(json);
    list->mJson = json;
    list->mRawPurpose = xjson.getRequiredString("purpose");
    list->mPurpose = stringToListPurpose(list->mRawPurpose);
    list->mName = xjson.getRequiredString("name");
    list->mDescription = xjson.getOptionalString("description");
    list->mDescriptionFacets = xjson.getOptionalVector<AppBskyRichtext::Facet>("descriptionFacets");
    list->mAvatar = xjson.getOptionalObject<Blob>("avatar");
    list->mLabels = xjson.getOptionalObject<ComATProtoLabel::SelfLabels>("labels");
    list->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return list;
}

QJsonObject ListBlock::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", "app.bsky.graph.listblock");
    json.insert("subject", mSubject);
    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));
    return json;
}

ListBlock::Ptr ListBlock::fromJson(const QJsonObject& json)
{
    auto listBlock = std::make_unique<ListBlock>();
    XJsonObject xjson(json);
    listBlock->mJson = json;
    listBlock->mSubject = xjson.getRequiredString("subject");
    listBlock->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return listBlock;
}

QJsonObject ListItem::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", "app.bsky.graph.listitem");
    json.insert("subject", mSubject);
    json.insert("list", mList);
    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));
    return json;
}

ListItem::Ptr ListItem::fromJson(const QJsonObject& json)
{
    auto listItem = std::make_unique<ListItem>();
    XJsonObject xjson(json);
    listItem->mJson = json;
    listItem->mSubject = xjson.getRequiredString("subject");
    listItem->mList = xjson.getRequiredString("list");
    listItem->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return listItem;
}

GetListOutput::Ptr GetListOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_unique<GetListOutput>();
    XJsonObject xjson(json);
    output->mCursor = xjson.getOptionalString("cursor");
    output->mList = xjson.getRequiredObject<ListView>("list");
    output->mItems = xjson.getRequiredVector<ListItemView>("items");
    return output;
}

GetListsOutput::Ptr GetListsOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_unique<GetListsOutput>();
    XJsonObject xjson(json);
    output->mCursor = xjson.getOptionalString("cursor");
    output->mLists = xjson.getRequiredVector<ListView>("lists");
    return output;
}

StarterPackFeedItem::Ptr StarterPackFeedItem::fromJson(const QJsonObject& json)
{
    auto feedItem = std::make_unique<StarterPackFeedItem>();
    XJsonObject xjson(json);
    feedItem->mUri = xjson.getRequiredString("uri");
    return feedItem;
}

StarterPack::Ptr StarterPack::fromJson(const QJsonObject& json)
{
    auto starterPack = std::make_unique<StarterPack>();
    XJsonObject xjson(json);
    starterPack->mName = xjson.getRequiredString("name");
    starterPack->mDescription = xjson.getOptionalString("description");
    starterPack->mDescriptionFacets = xjson.getOptionalVector<AppBskyRichtext::Facet>("descriptionFacets");
    starterPack->mList = xjson.getRequiredString("list");
    starterPack->mFeeds = xjson.getOptionalVector<StarterPackFeedItem>("feeds");
    starterPack->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return starterPack;
}

StarterPackViewBasic::Ptr StarterPackViewBasic::fromJson(const QJsonObject& json)
{
    auto view = std::make_unique<StarterPackViewBasic>();
    XJsonObject xjson(json);
    view->mUri = xjson.getRequiredString("uri");
    view->mCid = xjson.getRequiredString("cid");
    view->mRecord = xjson.getRequiredVariant<StarterPack>("record");
    view->mCreator = xjson.getRequiredObject<AppBskyActor::ProfileViewBasic>("creator");
    view->mListItemCount = xjson.getOptionalInt("listItemCount", 0);
    view->mJoinedWeekCount = xjson.getOptionalInt("joinedWeekCount", 0);
    view->mJoinedAllTimeCount = xjson.getOptionalInt("joinedAllTimeCount", 0);
    ComATProtoLabel::getLabels(view->mLabels, json);
    view->mIndexedAt = xjson.getRequiredDateTime("indexedAt");
    return view;
}

StarterPackView::Ptr StarterPackView::fromJson(const QJsonObject& json)
{
    auto view = std::make_unique<StarterPackView>();
    XJsonObject xjson(json);
    view->mUri = xjson.getRequiredString("uri");
    view->mCid = xjson.getRequiredString("cid");
    view->mRecord = xjson.getRequiredVariant<StarterPack>("record");
    view->mCreator = xjson.getRequiredObject<AppBskyActor::ProfileViewBasic>("creator");
    view->mList = xjson.getOptionalObject<ListViewBasic>("list");
    view->mListItemsSample = xjson.getOptionalVector<ListItemView>("listItemsSample");
    view->mFeeds = xjson.getOptionalVector<AppBskyFeed::GeneratorView>("feeds");
    view->mJoinedWeekCount = xjson.getOptionalInt("joinedWeekCount", 0);
    view->mJoinedAllTimeCount = xjson.getOptionalInt("joinedAllTimeCount", 0);
    ComATProtoLabel::getLabels(view->mLabels, json);
    view->mIndexedAt = xjson.getRequiredDateTime("indexedAt");
    return view;
}

GetStarterPacksOutput::Ptr GetStarterPacksOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_unique<GetStarterPacksOutput>();
    XJsonObject xjson(json);
    output->mCursor = xjson.getOptionalString("cursor");
    output->mStarterPacks = xjson.getRequiredVector<StarterPackViewBasic>("starterPacks");
    return output;
}

GetStarterPackOutput::Ptr GetStarterPackOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_unique<GetStarterPackOutput>();
    XJsonObject xjson(json);
    output->mStarterPack = xjson.getRequiredObject<StarterPackView>("starterPack");
    return output;
}

}
