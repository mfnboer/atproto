// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>

// Extra header to break cyclic dependencies

namespace ATProto::AppBskyGraph {

// app.bsky.graph.defs#listViewerState
struct ListViewerState
{
    bool mMuted = false;
    std::optional<QString> mBlocked; // at-uri

    using Ptr = std::unique_ptr<ListViewerState>;
    static Ptr fromJson(const QJsonObject& json);
};

enum class ListPurpose
{
    MOD_LIST,
    CURATE_LIST,
    UNKNOWN
};

// app.bsky.graph.defs#listViewBasic
struct ListViewBasic
{
    QString mUri;
    QString mCid;
    QString mName;
    ListPurpose mPurpose;
    QString mRawPurpose;
    std::optional<QString> mAvatar;
    ListViewerState::Ptr mViewer; // optional
    std::optional<QDateTime> mIndexedAt;

    using SharedPtr = std::shared_ptr<ListViewBasic>;
    using Ptr = std::unique_ptr<ListViewBasic>;
    static Ptr fromJson(const QJsonObject& json);
};

}
