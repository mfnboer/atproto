// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include "com_atproto_label.h"

namespace ATProto::AppBskyLabeler {

// app.bsky.labeler.defs#labelerViewerState
struct LabelerViewerState
{
    std::optional<QString> mLike;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<LabelerViewerState>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.labeler.defs#labelerPolicies
struct LabelerPolicies
{
    std::vector<QString> mLabelValues; // com.atproto.label.defs#labelValue
    ComATProtoLabel::LabelValueDefinition::List mLabelValueDefinitions;

    using SharedPtr = std::shared_ptr<LabelerPolicies>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.labeler.defs#labelerView
struct LabelerView
{
    QString mUri;
    QString mCid;
    AppBskyActor::ProfileView::SharedPtr mCreator;
    int mLikeCount = 0;
    LabelerViewerState::SharedPtr mViewer; // optional
    QDateTime mIndexedAt;
    ComATProtoLabel::Label::List mLabels;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<LabelerView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.labeler.defs#labelerView";
};

// app.bsky.labeler.defs#labelerViewDetailed
struct LabelerViewDetailed
{
    QString mUri;
    QString mCid;
    AppBskyActor::ProfileView::SharedPtr mCreator;
    LabelerPolicies::SharedPtr mPolicies;
    int mLikeCount = 0;
    LabelerViewerState::SharedPtr mViewer; // optional
    QDateTime mIndexedAt;
    ComATProtoLabel::Label::List mLabels;

    using SharedPtr = std::shared_ptr<LabelerViewDetailed>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct GetServicesOutputView
{
    enum class ViewType
    {
        VIEW,
        VIEW_DETAILED,
        UNKNOWN
    };
    static ViewType stringToViewType(const QString& str);

    std::variant<LabelerView::SharedPtr, LabelerViewDetailed::SharedPtr> mView;
    ViewType mViewType = ViewType::UNKNOWN;
    QString mRawViewType;

    using SharedPtr = std::shared_ptr<GetServicesOutputView>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.labeler.getServices#output
struct GetServicesOutput
{
    GetServicesOutputView::List mViews;

    using SharedPtr = std::shared_ptr<GetServicesOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}
