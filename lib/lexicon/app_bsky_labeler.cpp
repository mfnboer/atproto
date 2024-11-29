// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "app_bsky_labeler.h"
#include "../xjson.h"

namespace ATProto::AppBskyLabeler {

QJsonObject LabelerViewerState::toJson() const
{
    QJsonObject json;
    XJsonObject::insertOptionalJsonValue(json, "like", mLike);
    return json;
}

LabelerViewerState::SharedPtr LabelerViewerState::fromJson(const QJsonObject& json)
{
    auto state = std::make_shared<LabelerViewerState>();
    XJsonObject xjson(json);
    state->mLike = xjson.getOptionalString("like");
    return state;
}

LabelerPolicies::SharedPtr LabelerPolicies::fromJson(const QJsonObject& json)
{
    auto policies = std::make_shared<LabelerPolicies>();
    XJsonObject xjson(json);
    policies->mLabelValues = xjson.getRequiredStringVector("labelValues");
    policies->mLabelValueDefinitions = xjson.getOptionalVector<ComATProtoLabel::LabelValueDefinition>("labelValueDefinitions");
    return policies;
}

QJsonObject LabelerView::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("uri", mUri);
    json.insert("cid", mCid);
    json.insert("creator", mCreator->toJson());
    XJsonObject::insertOptionalJsonValue(json, "likeCount", mLikeCount, 0);
    XJsonObject::insertOptionalJsonObject<LabelerViewerState>(json, "viewer", mViewer);
    json.insert("indexedAt", mIndexedAt.toString(Qt::ISODateWithMs));
    XJsonObject::insertOptionalArray<ComATProtoLabel::Label>(json, "labels", mLabels);
    return json;
}

LabelerView::SharedPtr LabelerView::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<LabelerView>();
    XJsonObject xjson(json);
    view->mUri = xjson.getRequiredString("uri");
    view->mCid = xjson.getRequiredString("cid");
    view->mCreator = xjson.getRequiredObject<AppBskyActor::ProfileView>("creator");
    view->mLikeCount = xjson.getOptionalInt("likeCount", 0);
    view->mViewer = xjson.getOptionalObject<LabelerViewerState>("viewer");
    view->mIndexedAt = xjson.getRequiredDateTime("indexedAt");
    view->mLabels = xjson.getOptionalVector<ComATProtoLabel::Label>("labels");
    return view;
}

LabelerViewDetailed::SharedPtr LabelerViewDetailed::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<LabelerViewDetailed>();
    XJsonObject xjson(json);
    view->mUri = xjson.getRequiredString("uri");
    view->mCid = xjson.getRequiredString("cid");
    view->mCreator = xjson.getRequiredObject<AppBskyActor::ProfileView>("creator");
    view->mPolicies = xjson.getRequiredObject<LabelerPolicies>("policies");
    view->mLikeCount = xjson.getOptionalInt("likeCount", 0);
    view->mViewer = xjson.getOptionalObject<LabelerViewerState>("viewer");
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

GetServicesOutputView::SharedPtr GetServicesOutputView::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<GetServicesOutputView>();
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

GetServicesOutput::SharedPtr GetServicesOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetServicesOutput>();
    XJsonObject xjson(json);
    output->mViews = xjson.getRequiredVector<GetServicesOutputView>("views");
    return output;
}

}
