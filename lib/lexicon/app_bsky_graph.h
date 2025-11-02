// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include "app_bsky_graph_include.h"
#include "app_bsky_richtext.h"
#include "com_atproto_label.h"
#include <QJsonDocument>

namespace ATProto::AppBskyGraph {

// app.bsky.graph.getFollows#output
struct GetFollowsOutput
{
    AppBskyActor::ProfileView::SharedPtr mSubject;
    AppBskyActor::ProfileView::List mFollows;
    std::optional<QString> mCursor;

    using SharedPtr = std::shared_ptr<GetFollowsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.getFollowers#output
struct GetFollowersOutput
{
    AppBskyActor::ProfileView::SharedPtr mSubject;
    AppBskyActor::ProfileView::List mFollowers;
    std::optional<QString> mCursor;

    using SharedPtr = std::shared_ptr<GetFollowersOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.getBlocks#output
struct GetBlocksOutput
{
    AppBskyActor::ProfileView::List mBlocks;
    std::optional<QString> mCursor;

    using SharedPtr = std::shared_ptr<GetBlocksOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.getMutes#output
struct GetMutesOutput
{
    AppBskyActor::ProfileView::List mMutes;
    std::optional<QString> mCursor;

    using SharedPtr = std::shared_ptr<GetMutesOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.follow
struct Follow
{
    QString mSubject; // did
    QDateTime mCreatedAt;
    ComATProtoRepo::StrongRef::SharedPtr mVia; // optional
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
    AppBskyRichtext::Facet::List mDescriptionFacets;
    std::optional<QString> mAvatar;
    ComATProtoLabel::Label::List mLabels;
    ListViewerState::SharedPtr mViewer; // optional
    std::optional<QDateTime> mIndexedAt;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ListView>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.defs#listItemView
struct ListItemView
{
    QString mUri;
    AppBskyActor::ProfileView::SharedPtr mSubject; // required

    using SharedPtr = std::shared_ptr<ListItemView>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.list
struct List
{
    ListPurpose mPurpose;
    QString mRawPurpose;
    QString mName;
    std::optional<QString> mDescription; // max 300 graphemes, 3000 bytes
    AppBskyRichtext::Facet::List mDescriptionFacets;
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
    ListItemView::List mItems;

    using SharedPtr = std::shared_ptr<GetListOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.getLists#output
struct GetListsOutput
{
    std::optional<QString> mCursor;
    ListView::List mLists;

    using SharedPtr = std::shared_ptr<GetListsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.getListsWithMembership#listWithMembership
struct ListWithMembership
{
    ListView::SharedPtr mList; // required
    ListItemView::SharedPtr mListItem; // optional

    using SharedPtr = std::shared_ptr<ListWithMembership>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.getListsWithMembership#output
struct GetListsWithMembershipOutput
{
    std::optional<QString> mCursor;
    ListWithMembership::List mListsWithMembership;

    using SharedPtr = std::shared_ptr<GetListsWithMembershipOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.starterpack#feedItem
struct StarterPackFeedItem
{
    QString mUri; // at-uri

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<StarterPackFeedItem>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.starterpack
struct StarterPack
{
    QString mName; // max_graphemes=50 max_bytes=500 min_bytes=1
    std::optional<QString> mDescription; // max_graphemes=300 max_bytes=3000
    AppBskyRichtext::Facet::List mDescriptionFacets;
    QString mList; // at-uri
    StarterPackFeedItem::List mFeeds;
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
    ComATProtoLabel::Label::List mLabels;
    QDateTime mIndexedAt;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<StarterPackViewBasic>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.graph.defs#starterPackViewBasic";
};

// app.bsky.graph.getStarterPacks#output
struct GetStarterPacksOutput
{
    StarterPackViewBasic::List mStarterPacks;
    std::optional<QString> mCursor;

    using SharedPtr = std::shared_ptr<GetStarterPacksOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.verification
struct Verification {
    QString mSubject; // DID
    QString mHandle;
    QString mDisplayName;
    QDateTime mCreatedAt;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Verification>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.graph.verification";
};

}
