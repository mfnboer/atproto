// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "com_atproto_label.h"
#include <QJsonDocument>

// Extra header to break cyclic dependencies

namespace ATProto::AppBskyGraph {

// app.bsky.graph.defs#listViewerState
struct ListViewerState
{
    bool mMuted = false;
    std::optional<QString> mBlocked; // at-uri

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ListViewerState>;
    static SharedPtr fromJson(const QJsonObject& json);
};

enum class ListPurpose
{
    MOD_LIST,
    CURATE_LIST,
    REFERENCE_LIST,
    UNKNOWN
};

// app.bsky.graph.defs#listViewBasic
struct ListViewBasic
{
    QString mUri;
    QString mCid;
    QString mName;
    ListPurpose mPurpose = ListPurpose::UNKNOWN;
    QString mRawPurpose;
    std::optional<QString> mAvatar;
    ComATProtoLabel::Label::List mLabels;
    ListViewerState::SharedPtr mViewer; // optional
    std::optional<QDateTime> mIndexedAt;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ListViewBasic>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}
