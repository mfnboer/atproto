// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_graph.h"
#include "app_bsky_embed.h"
#include "../xjson.h"
#include <unordered_map>

namespace ATProto::AppBskyGraph {

GetFollowsOutput::SharedPtr GetFollowsOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto follows = std::make_shared<GetFollowsOutput>();
    follows->mSubject = xjson.getRequiredObject<AppBskyActor::ProfileView>("subject");
    follows->mFollows = xjson.getRequiredVector<AppBskyActor::ProfileView>("follows");
    follows->mCursor = xjson.getOptionalString("cursor");
    return follows;
}

GetFollowersOutput::SharedPtr GetFollowersOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto followers = std::make_shared<GetFollowersOutput>();
    followers->mSubject = xjson.getRequiredObject<AppBskyActor::ProfileView>("subject");
    followers->mFollowers = xjson.getRequiredVector<AppBskyActor::ProfileView>("followers");
    followers->mCursor = xjson.getOptionalString("cursor");
    return followers;
}

GetBlocksOutput::SharedPtr GetBlocksOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto blocks = std::make_shared<GetBlocksOutput>();
    blocks->mBlocks = xjson.getRequiredVector<AppBskyActor::ProfileView>("blocks");
    blocks->mCursor = xjson.getOptionalString("cursor");
    return blocks;
}

GetMutesOutput::SharedPtr GetMutesOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto blocks = std::make_shared<GetMutesOutput>();
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

Follow::SharedPtr Follow::fromJson(const QJsonObject& json)
{
    auto follow = std::make_shared<Follow>();
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

Block::SharedPtr Block::fromJson(const QJsonObject& json)
{
    auto block = std::make_shared<Block>();
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

QJsonObject ListViewerState::toJson() const
{
    QJsonObject json;
    XJsonObject::insertOptionalJsonValue(json, "muted", mMuted, false);
    XJsonObject::insertOptionalJsonValue(json, "blocked", mBlocked);

    return json;
}

ListViewerState::SharedPtr ListViewerState::fromJson(const QJsonObject& json)
{
    auto viewerState = std::make_shared<ListViewerState>();
    XJsonObject xjson(json);
    viewerState->mMuted = xjson.getOptionalBool("muted", false);
    viewerState->mBlocked = xjson.getOptionalString("blocked");
    return viewerState;
}

QJsonObject ListViewBasic::toJson() const
{
    QJsonObject json;
    json.insert("uri", mUri);
    json.insert("cid", mCid);
    json.insert("name", mName);
    json.insert("purpose", mRawPurpose);
    XJsonObject::insertOptionalJsonValue(json, "avatar", mAvatar);
    XJsonObject::insertOptionalArray<ComATProtoLabel::Label>(json, "labels", mLabels);
    XJsonObject::insertOptionalJsonObject<ListViewerState>(json, "viewer", mViewer);
    XJsonObject::insertOptionalDateTime(json, "indexedAt", mIndexedAt);

    return json;
}

ListViewBasic::SharedPtr ListViewBasic::fromJson(const QJsonObject& json)
{
    auto listView = std::make_shared<ListViewBasic>();
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
    XJsonObject::insertOptionalArray<AppBskyRichtext::Facet>(json, "descriptionFacets", mDescriptionFacets);
    XJsonObject::insertOptionalArray<ComATProtoLabel::Label>(json, "labels", mLabels);
    XJsonObject::insertOptionalJsonObject<ListViewerState>(json, "viewer", mViewer);
    XJsonObject::insertOptionalDateTime(json, "indexedAt", mIndexedAt);
    return json;
}

ListView::SharedPtr ListView::fromJson(const QJsonObject& json)
{
    auto listView = std::make_shared<ListView>();
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

ListItemView::SharedPtr ListItemView::fromJson(const QJsonObject& json)
{
    auto listItemView = std::make_shared<ListItemView>();
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

List::SharedPtr List::fromJson(const QJsonObject& json)
{
    auto list = std::make_shared<List>();
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

ListBlock::SharedPtr ListBlock::fromJson(const QJsonObject& json)
{
    auto listBlock = std::make_shared<ListBlock>();
    XJsonObject xjson(json);
    listBlock->mJson = json;
    listBlock->mSubject = xjson.getRequiredString("subject");
    listBlock->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return listBlock;
}

QJsonObject ListItem::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", TYPE);
    json.insert("subject", mSubject);
    json.insert("list", mList);
    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));
    return json;
}

ListItem::SharedPtr ListItem::fromJson(const QJsonObject& json)
{
    auto listItem = std::make_shared<ListItem>();
    XJsonObject xjson(json);
    listItem->mJson = json;
    listItem->mSubject = xjson.getRequiredString("subject");
    listItem->mList = xjson.getRequiredString("list");
    listItem->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return listItem;
}

GetListOutput::SharedPtr GetListOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetListOutput>();
    XJsonObject xjson(json);
    output->mCursor = xjson.getOptionalString("cursor");
    output->mList = xjson.getRequiredObject<ListView>("list");
    output->mItems = xjson.getRequiredVector<ListItemView>("items");
    return output;
}

GetListsOutput::SharedPtr GetListsOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetListsOutput>();
    XJsonObject xjson(json);
    output->mCursor = xjson.getOptionalString("cursor");
    output->mLists = xjson.getRequiredVector<ListView>("lists");
    return output;
}

ListWithMembership::SharedPtr ListWithMembership::fromJson(const QJsonObject& json)
{
    auto list = std::make_shared<ListWithMembership>();
    XJsonObject xjson(json);
    list->mList = xjson.getRequiredObject<ListView>("list");
    list->mListItem = xjson.getOptionalObject<ListItemView>("listItem");
    return list;
}

GetListsWithMembershipOutput::SharedPtr GetListsWithMembershipOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetListsWithMembershipOutput>();
    XJsonObject xjson(json);
    output->mCursor = xjson.getOptionalString("cursor");
    output->mListsWithMembership = xjson.getRequiredVector<ListWithMembership>("listsWithMembership");
    return output;
}

QJsonObject StarterPackFeedItem::toJson() const
{
    QJsonObject json;
    json.insert("uri", mUri);
    return json;
}

StarterPackFeedItem::SharedPtr StarterPackFeedItem::fromJson(const QJsonObject& json)
{
    auto feedItem = std::make_shared<StarterPackFeedItem>();
    XJsonObject xjson(json);
    feedItem->mUri = xjson.getRequiredString("uri");
    return feedItem;
}

QJsonObject StarterPack::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("name", mName);
    XJsonObject::insertOptionalJsonValue(json, "description", mDescription);
    XJsonObject::insertOptionalArray<AppBskyRichtext::Facet>(json, "descriptionFacets", mDescriptionFacets);
    json.insert("list", mList);
    XJsonObject::insertOptionalArray<StarterPackFeedItem>(json, "feeds", mFeeds);
    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));
    return json;
}

StarterPack::SharedPtr StarterPack::fromJson(const QJsonObject& json)
{
    auto starterPack = std::make_shared<StarterPack>();
    XJsonObject xjson(json);
    starterPack->mName = xjson.getRequiredString("name");
    starterPack->mDescription = xjson.getOptionalString("description");
    starterPack->mDescriptionFacets = xjson.getOptionalVector<AppBskyRichtext::Facet>("descriptionFacets");
    starterPack->mList = xjson.getRequiredString("list");
    starterPack->mFeeds = xjson.getOptionalVector<StarterPackFeedItem>("feeds");
    starterPack->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return starterPack;
}

QJsonObject StarterPackViewBasic::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("uri", mUri);
    json.insert("cid", mCid);
    json.insert("record", XJsonObject::variantToJsonObject(mRecord));
    json.insert("creator", mCreator->toJson());
    XJsonObject::insertOptionalJsonValue(json, "listItemCount", mListItemCount, 0);
    XJsonObject::insertOptionalJsonValue(json, "joinedWeekCount", mJoinedWeekCount, 0);
    XJsonObject::insertOptionalJsonValue(json, "joinedAllTimeCount", mJoinedAllTimeCount, 0);
    XJsonObject::insertOptionalArray<ComATProtoLabel::Label>(json, "labels", mLabels);
    json.insert("indexedAt", mIndexedAt.toString(Qt::ISODateWithMs));
    return json;
}

StarterPackViewBasic::SharedPtr StarterPackViewBasic::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<StarterPackViewBasic>();
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

StarterPackView::SharedPtr StarterPackView::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<StarterPackView>();
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

GetStarterPacksOutput::SharedPtr GetStarterPacksOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetStarterPacksOutput>();
    XJsonObject xjson(json);
    output->mCursor = xjson.getOptionalString("cursor");
    output->mStarterPacks = xjson.getRequiredVector<StarterPackViewBasic>("starterPacks");
    return output;
}

GetStarterPackOutput::SharedPtr GetStarterPackOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetStarterPackOutput>();
    XJsonObject xjson(json);
    output->mStarterPack = xjson.getRequiredObject<StarterPackView>("starterPack");
    return output;
}

StarterPackWithMembership::SharedPtr StarterPackWithMembership::fromJson(const QJsonObject& json)
{
    auto starterPack = std::make_shared<StarterPackWithMembership>();
    XJsonObject xjson(json);
    starterPack->mStarterPack = xjson.getRequiredObject<StarterPackView>("starterPack");
    starterPack->mListItem = xjson.getOptionalObject<ListItemView>("listItem");
    return starterPack;
}

GetStarterPacksWithMembershipOutput::SharedPtr GetStarterPacksWithMembershipOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetStarterPacksWithMembershipOutput>();
    XJsonObject xjson(json);
    output->mCursor = xjson.getOptionalString("cursor");
    output->mStarterPacksWithMembership = xjson.getRequiredVector<StarterPackWithMembership>("starterPacksWithMembership");
    return output;
}

QJsonObject Verification::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", TYPE);
    json.insert("subject", mSubject);
    json.insert("handle", mHandle);
    json.insert("displayName", mDisplayName);
    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));
    return json;
}

Verification::SharedPtr Verification::fromJson(const QJsonObject& json)
{
    auto verification = std::make_shared<Verification>();
    XJsonObject xjson(json);
    verification->mJson = json;
    verification->mSubject = xjson.getRequiredString("subject");
    verification->mHandle = xjson.getRequiredString("handle");
    verification->mDisplayName = xjson.getRequiredString("displayName");
    verification->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return verification;
}

}
