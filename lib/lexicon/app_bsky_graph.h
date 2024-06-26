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
    AppBskyActor::ProfileView::Ptr mSubject;
    AppBskyActor::ProfileViewList mFollows;
    std::optional<QString> mCursor;

    using Ptr = std::unique_ptr<GetFollowsOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.graph.getFollowers/output
struct GetFollowersOutput
{
    AppBskyActor::ProfileView::Ptr mSubject;
    AppBskyActor::ProfileViewList mFollowers;
    std::optional<QString> mCursor;

    using Ptr = std::unique_ptr<GetFollowersOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.graph.getBlocks/output
struct GetBlocksOutput
{
    AppBskyActor::ProfileViewList mBlocks;
    std::optional<QString> mCursor;

    using Ptr = std::unique_ptr<GetBlocksOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.graph.getMutes/output
struct GetMutesOutput
{
    AppBskyActor::ProfileViewList mMutes;
    std::optional<QString> mCursor;

    using Ptr = std::unique_ptr<GetMutesOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.graph.follow
struct Follow
{
    QString mSubject; // did
    QDateTime mCreatedAt;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Follow>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.graph.block
struct Block
{
    QString mSubject; // did
    QDateTime mCreatedAt;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Block>;
    static Ptr fromJson(const QJsonObject& json);
};

ListPurpose stringToListPurpose(const QString& str);
QString listPurposeToString(ListPurpose purpose);

using ListViewBasicList = std::vector<ListViewBasic::Ptr>;

// app.bsky.graph.defs#listView
struct ListView
{
    QString mUri;
    QString mCid;
    AppBskyActor::ProfileView::Ptr mCreator; // required
    QString mName;
    ListPurpose mPurpose;
    QString mRawPurpose;
    std::optional<QString> mDescription; // max 300 graphemes, 3000 bytes
    AppBskyRichtext::FacetList mDescriptionFacets;
    std::optional<QString> mAvatar;
    std::vector<ComATProtoLabel::Label::Ptr> mLabels;
    ListViewerState::Ptr mViewer; // optional
    std::optional<QDateTime> mIndexedAt;

    QJsonObject toJson() const; // partial serialization

    using SharedPtr = std::shared_ptr<ListView>;
    using Ptr = std::unique_ptr<ListView>;
    static Ptr fromJson(const QJsonObject& json);
};

using ListViewList = std::vector<ListView::Ptr>;

// app.bsky.graph.defs#listItemView
struct ListItemView
{
    QString mUri;
    AppBskyActor::ProfileView::Ptr mSubject; // required

    using Ptr = std::unique_ptr<ListItemView>;
    static Ptr fromJson(const QJsonObject& json);
};

using ListItemViewList = std::vector<ListItemView::Ptr>;

// app.bsky.graph.list
struct List
{
    ListPurpose mPurpose;
    QString mRawPurpose;
    QString mName;
    std::optional<QString> mDescription; // max 300 graphemes, 3000 bytes
    AppBskyRichtext::FacetList mDescriptionFacets;
    Blob::Ptr mAvatar; // optional
    ComATProtoLabel::SelfLabels::Ptr mLabels; // optional
    QDateTime mCreatedAt;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<List>;
    using Ptr = std::unique_ptr<List>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.graph.listBlock
struct ListBlock
{
    QString mSubject; // at-uri
    QDateTime mCreatedAt;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<ListBlock>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.graph.listitem
struct ListItem
{
    QString mSubject; // did
    QString mList; // at-uri
    QDateTime mCreatedAt;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<ListItem>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.graph.getList#output
struct GetListOutput
{
    std::optional<QString> mCursor;
    ListView::Ptr mList; // required
    ListItemViewList mItems;

    using Ptr = std::unique_ptr<GetListOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.graph.getLists#output
struct GetListsOutput
{
    std::optional<QString> mCursor;
    ListViewList mLists;

    using Ptr = std::unique_ptr<GetListsOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

}
