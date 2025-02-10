// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include "app_bsky_graph_include.h"
#include "app_bsky_richtext.h"
#include "com_atproto_label.h"
#include <QJsonDocument>

namespace ATProto::AppBskyGraph {

// app.bsky.graph.getFollows/output
struct GetFollowsOutput
{
    AppBskyActor::ProfileView::SharedPtr mSubject;
    AppBskyActor::ProfileViewList mFollows;
    std::optional<QString> mCursor;

    using SharedPtr = std::shared_ptr<GetFollowsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.getFollowers/output
struct GetFollowersOutput
{
    AppBskyActor::ProfileView::SharedPtr mSubject;
    AppBskyActor::ProfileViewList mFollowers;
    std::optional<QString> mCursor;

    using SharedPtr = std::shared_ptr<GetFollowersOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.getBlocks/output
struct GetBlocksOutput
{
    AppBskyActor::ProfileViewList mBlocks;
    std::optional<QString> mCursor;

    using SharedPtr = std::shared_ptr<GetBlocksOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.getMutes/output
struct GetMutesOutput
{
    AppBskyActor::ProfileViewList mMutes;
    std::optional<QString> mCursor;

    using SharedPtr = std::shared_ptr<GetMutesOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.follow
struct Follow
{
    QString mSubject; // did
    QDateTime mCreatedAt;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Follow>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.block
struct Block
{
    QString mSubject; // did
    QDateTime mCreatedAt;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Block>;
    static SharedPtr fromJson(const QJsonObject& json);
};

ListPurpose stringToListPurpose(const QString& str);
QString listPurposeToString(ListPurpose purpose);

using ListViewBasicList = std::vector<ListViewBasic::SharedPtr>;

// app.bsky.graph.defs#listView
struct ListView
{
    QString mUri;
    QString mCid;
    AppBskyActor::ProfileView::SharedPtr mCreator; // required
    QString mName;
    ListPurpose mPurpose;
    QString mRawPurpose;
    std::optional<QString> mDescription; // max 300 graphemes, 3000 bytes
    AppBskyRichtext::FacetList mDescriptionFacets;
    std::optional<QString> mAvatar;
    ComATProtoLabel::LabelList mLabels;
    ListViewerState::SharedPtr mViewer; // optional
    std::optional<QDateTime> mIndexedAt;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ListView>;
    static SharedPtr fromJson(const QJsonObject& json);
};

using ListViewList = std::vector<ListView::SharedPtr>;

// app.bsky.graph.defs#listItemView
struct ListItemView
{
    QString mUri;
    AppBskyActor::ProfileView::SharedPtr mSubject; // required

    using SharedPtr = std::shared_ptr<ListItemView>;
    static SharedPtr fromJson(const QJsonObject& json);
};

using ListItemViewList = std::vector<ListItemView::SharedPtr>;

// app.bsky.graph.list
struct List
{
    ListPurpose mPurpose;
    QString mRawPurpose;
    QString mName;
    std::optional<QString> mDescription; // max 300 graphemes, 3000 bytes
    AppBskyRichtext::FacetList mDescriptionFacets;
    Blob::SharedPtr mAvatar; // optional
    ComATProtoLabel::SelfLabels::SharedPtr mLabels; // optional
    QDateTime mCreatedAt;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<List>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.listBlock
struct ListBlock
{
    QString mSubject; // at-uri
    QDateTime mCreatedAt;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ListBlock>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.listitem
struct ListItem
{
    QString mSubject; // did
    QString mList; // at-uri
    QDateTime mCreatedAt;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ListItem>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.graph.listitem";
};

// app.bsky.graph.getList#output
struct GetListOutput
{
    std::optional<QString> mCursor;
    ListView::SharedPtr mList; // required
    ListItemViewList mItems;

    using SharedPtr = std::shared_ptr<GetListOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.getLists#output
struct GetListsOutput
{
    std::optional<QString> mCursor;
    ListViewList mLists;

    using SharedPtr = std::shared_ptr<GetListsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.starterpack#feedItem
struct StarterPackFeedItem
{
    QString mUri; // at-uri

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<StarterPackFeedItem>;
    static SharedPtr fromJson(const QJsonObject& json);
};
using StarterPackFeedItemList = std::vector<StarterPackFeedItem::SharedPtr>;

// app.bsky.graph.starterpack
struct StarterPack
{
    QString mName; // max_graphemes=50 max_bytes=500 min_bytes=1
    std::optional<QString> mDescription; // max_graphemes=300 max_bytes=3000
    AppBskyRichtext::FacetList mDescriptionFacets;
    QString mList; // at-uri
    StarterPackFeedItemList mFeeds;
    QDateTime mCreatedAt;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<StarterPack>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.graph.starterpack";
};

// app.bsky.graph.defs#starterPackViewBasic
struct StarterPackViewBasic
{
    QString mUri; // at-uri
    QString mCid;
    std::variant<StarterPack::SharedPtr> mRecord; // null variant for unknown type
    AppBskyActor::ProfileViewBasic::SharedPtr mCreator;
    int mListItemCount = 0;
    int mJoinedWeekCount = 0;
    int mJoinedAllTimeCount = 0;
    ComATProtoLabel::LabelList mLabels;
    QDateTime mIndexedAt;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<StarterPackViewBasic>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.graph.defs#starterPackViewBasic";
};
using StarterPackViewBasicList = std::vector<StarterPackViewBasic::SharedPtr>;

// app.bsky.graph.getStarterPacks/output
struct GetStarterPacksOutput
{
    StarterPackViewBasicList mStarterPacks;
    std::optional<QString> mCursor;

    using SharedPtr = std::shared_ptr<GetStarterPacksOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}
