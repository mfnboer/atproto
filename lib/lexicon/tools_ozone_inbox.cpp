// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "tools_ozone_inbox.h"
#include "../xjson.h"

namespace ATProto {

template<>
std::unordered_map<QString, ToolsOzoneInbox::ReportScopeEnum> ToolsOzoneInbox::ReportScope::sStringToEnumMap = {
    { "network", ToolsOzoneInbox::ReportScopeEnum::NETWORK },
    { "app", ToolsOzoneInbox::ReportScopeEnum::APP },
    { "labelOnly", ToolsOzoneInbox::ReportScopeEnum::LABEL_ONLY }
};

template<>
std::unordered_map<QString, ToolsOzoneInbox::ReportStatusEnum> ToolsOzoneInbox::ReportStatus::sStringToEnumMap = {
    { "pending", ToolsOzoneInbox::ReportStatusEnum::PENDING },
    { "resolved", ToolsOzoneInbox::ReportStatusEnum::RESOLVED }
};

template<>
std::unordered_map<QString, ToolsOzoneInbox::ResolutionOutcomeEnum> ToolsOzoneInbox::ResolutionOutcome::sStringToEnumMap = {
    { "actionTaken", ToolsOzoneInbox::ResolutionOutcomeEnum::ACTION_TAKEN },
    { "noAction", ToolsOzoneInbox::ResolutionOutcomeEnum::NO_ACTION },
    { "other", ToolsOzoneInbox::ResolutionOutcomeEnum::OTHER }
};

}

namespace ATProto::ToolsOzoneInbox {

ReportView::SharedPtr ReportView::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<ReportView>();
    XJsonObject xjson(json);
    view->mSrc = xjson.getRequiredString("src");
    view->mId = xjson.getRequiredInt("id");
    view->mIsRead = xjson.getOptionalBool("isRead", true);
    view->mReasonType = xjson.getRequiredString("reasonType");
    view->mReason = xjson.getOptionalString("reason");
    view->mLastActionTaken = xjson.getOptionalString("lastActionTaken");
    view->mScope = xjson.getOptionalEnum<ReportScope>("scope");
    view->mSubject = xjson.getRequiredVariant<ComATProtoAdmin::RepoRef,
                                              ComATProtoRepo::StrongRef,
                                              ChatBskyConvo::MessageRef,
                                              ChatBskyConvo::ConvoRef,
                                              UnknownVariant>("subject");
    view->mRecord = xjson.getOptionalVariant<AppBskyFeed::Record::Post,
                                             AppBskyGraph::List,
                                             AppBskyFeed::Generator,
                                             AppBskyGraph::StarterPack,
                                             AppBskyActor::Status,
                                             AppBskyActor::Link,
                                             UnknownVariant>("record");
    view->mStatus = xjson.getRequiredEnum<ReportStatus>("status");
    view->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    view->mUpdatedAt = xjson.getRequiredDateTime("updatedAt");
    return view;
}

ListReportsOutput::SharedPtr ListReportsOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<ListReportsOutput>();
    XJsonObject xjson(json);
    output->mCursor = xjson.getOptionalString("cursor");
    output->mReports = xjson.getRequiredVector<ReportView>("reports");
    return output;
}

ResolutionView::SharedPtr ResolutionView::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<ResolutionView>();
    XJsonObject xjson(json);
    view->mOutcome = xjson.getRequiredEnum<ResolutionOutcome>("outcome");
    view->mActionTaken = xjson.getOptionalString("actionTaken");
    view->mScope = xjson.getOptionalEnum<ReportScope>("scope");
    view->mResolvedAt = xjson.getRequiredDateTime("resolvedAt");
    return view;
}

GetReportOutput::SharedPtr GetReportOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetReportOutput>();
    XJsonObject xjson(json);
    output->mReport = xjson.getRequiredObject<ReportView>("report");
    output->mResolution = xjson.getOptionalObject<ResolutionView>("resolution");
    return output;
}

}