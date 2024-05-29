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

    using Ptr = std::unique_ptr<LabelerViewerState>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.labeler.defs#labelerPolicies
struct LabelerPolicies
{
    std::vector<QString> mLabelValues; // com.atproto.label.defs#labelValue
    std::vector<ComATProtoLabel::LabelValueDefinition::Ptr> mLabelValueDefinitions;

    using Ptr = std::unique_ptr<LabelerPolicies>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.labeler.defs#labelerView
struct LabelerView
{
    QString mUri;
    QString mCid;
    AppBskyActor::ProfileView::Ptr mCreator;
    int mLikeCount = 0;
    LabelerViewerState::Ptr mViewer; // optional
    QDateTime mIndexedAt;
    std::vector<ComATProtoLabel::Label::Ptr> mLabels;

    QJsonObject toJson() const; // partial serialization

    using Ptr = std::unique_ptr<LabelerView>;
    static Ptr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.labeler.defs#labelerView";
};

// app.bsky.labeler.defs#labelerViewDetailed
struct LabelerViewDetailed
{
    QString mUri;
    QString mCid;
    AppBskyActor::ProfileView::Ptr mCreator;
    LabelerPolicies::Ptr mPolicies;
    int mLikeCount = 0;
    LabelerViewerState::Ptr mViewer; // optional
    QDateTime mIndexedAt;
    std::vector<ComATProtoLabel::Label::Ptr> mLabels;

    using Ptr = std::unique_ptr<LabelerViewDetailed>;
    static Ptr fromJson(const QJsonObject& json);
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

    std::variant<LabelerView::Ptr, LabelerViewDetailed::Ptr> mView;
    ViewType mViewType;
    QString mRawViewType;

    using Ptr = std::unique_ptr<GetServicesOutputView>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.labeler.getServices#output
struct GetServicesOutput
{
    std::vector<GetServicesOutputView::Ptr> mViews;

    using Ptr = std::unique_ptr<GetServicesOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

}
