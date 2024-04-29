// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "app_bsky_labeler.h"
#include "../xjson.h"

namespace ATProto::AppBskyLabeler {

LabelerViewState::Ptr LabelerViewState::fromJson(const QJsonObject& json)
{
    auto state = std::make_unique<LabelerViewState>();
    XJsonObject xjson(json);
    state->mLike = xjson.getOptionalString("like");
    return state;
}

LabelerPolicies::Ptr LabelerPolicies::fromJson(const QJsonObject& json)
{
    auto policies = std::make_unique<LabelerPolicies>();
    XJsonObject xjson(json);
    policies->mLabelValues = xjson.getRequiredStringVector("labelValues");
    policies->mLabelValueDefinitions = xjson.getOptionalVector<ComATProtoLabel::LabelValueDefinition>("labelValueDefinitions");
    return policies;
}

LabelerView::Ptr LabelerView::fromJson(const QJsonObject& json)
{
    auto view = std::make_unique<LabelerView>();
    XJsonObject xjson(json);
    view->mUri = xjson.getRequiredString("uri");
    view->mCid = xjson.getRequiredString("cid");
    view->mCreator = xjson.getRequiredObject<AppBskyActor::ProfileView>("creator");
    view->mLikeCount = xjson.getOptionalInt("likeCount", 0);
    view->mViewer = xjson.getOptionalObject<LabelerViewState>("viewer");
    view->mIndexedAt = xjson.getRequiredDateTime("indexedAt");
    view->mLabels = xjson.getOptionalVector<ComATProtoLabel::Label>("labels");
    return view;
}

LabelerViewDetailed::Ptr LabelerViewDetailed::fromJson(const QJsonObject& json)
{
    auto view = std::make_unique<LabelerViewDetailed>();
    XJsonObject xjson(json);
    view->mUri = xjson.getRequiredString("uri");
    view->mCid = xjson.getRequiredString("cid");
    view->mCreator = xjson.getRequiredObject<AppBskyActor::ProfileView>("creator");
    view->mPolicies = xjson.getRequiredObject<LabelerPolicies>("policies");
    view->mLikeCount = xjson.getOptionalInt("likeCount", 0);
    view->mViewer = xjson.getOptionalObject<LabelerViewState>("viewer");
    view->mIndexedAt = xjson.getRequiredDateTime("indexedAt");
    view->mLabels = xjson.getOptionalVector<ComATProtoLabel::Label>("labels");
    return view;
}

GetServicesOutputView::ViewType GetServicesOutputView::stringToViewType(const QString& str)
{
    static const std::unordered_map<QString, ViewType> mapping = {
        { "app.bsky.labeler.defs#labelerView", ViewType::VIEW },
        { "app.bsky.labeler.defs#labelerViewDetailed", ViewType::VIEW_DETAILED }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    return ViewType::UNKNOWN;
}

GetServicesOutputView::Ptr GetServicesOutputView::fromJson(const QJsonObject& json)
{
    auto view = std::make_unique<GetServicesOutputView>();
    XJsonObject xjson(json);
    view->mRawViewType = xjson.getRequiredString("$type");
    view->mViewType = stringToViewType(view->mRawViewType);

    switch(view->mViewType)
    {
    case ViewType::VIEW:
        view->mView = LabelerView::fromJson(json);
        break;
    case ViewType::VIEW_DETAILED:
        view->mView = LabelerViewDetailed::fromJson(json);
        break;
    case ViewType::UNKNOWN:
        qWarning() << "Unsupported view type:" << view->mRawViewType;
        break;
    }

    return view;
}

GetServicesOutput::Ptr GetServicesOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_unique<GetServicesOutput>();
    XJsonObject xjson(json);
    output->mViews = xjson.getRequiredVector<GetServicesOutputView>("views");
    return output;
}

}
