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

template<>
std::unordered_map<QString, ToolsOzoneInbox::EnforcementStateEnum> ToolsOzoneInbox::EnforcementState::sStringToEnumMap = {
    { "none", ToolsOzoneInbox::EnforcementStateEnum::NONE },
    { "labeled", ToolsOzoneInbox::EnforcementStateEnum::LABELED },
    { "removed", ToolsOzoneInbox::EnforcementStateEnum::REMOVED },
    { "suspended", ToolsOzoneInbox::EnforcementStateEnum::SUSPENDED },
    { "takendown", ToolsOzoneInbox::EnforcementStateEnum::TAKENDOWN }
};

template<>
std::unordered_map<QString, ToolsOzoneInbox::AppealStateEnum> ToolsOzoneInbox::AppealState::sStringToEnumMap = {
    { "none", ToolsOzoneInbox::AppealStateEnum::NONE },
    { "pending", ToolsOzoneInbox::AppealStateEnum::PENDING },
    { "resolved", ToolsOzoneInbox::AppealStateEnum::RESOLVED },
    { "superseded", ToolsOzoneInbox::AppealStateEnum::SUPERSEDED },
    { "expired", ToolsOzoneInbox::AppealStateEnum::EXPIRED }
};

template<>
std::unordered_map<QString, ToolsOzoneInbox::AppealOutcomeEnum> ToolsOzoneInbox::AppealOutcome::sStringToEnumMap = {
    { "approved", ToolsOzoneInbox::AppealOutcomeEnum::APPROVED },
    { "denied", ToolsOzoneInbox::AppealOutcomeEnum::DENIED }
};

template<>
std::unordered_map<QString, ToolsOzoneInbox::SubjectActionEnum> ToolsOzoneInbox::SubjectAction::sStringToEnumMap = {
    { "appeal", ToolsOzoneInbox::SubjectActionEnum::APPEAL }
};

template<>
std::unordered_map<QString, ToolsOzoneInbox::AccountStandingEnum> ToolsOzoneInbox::AccountStanding::sStringToEnumMap = {
    { "good", ToolsOzoneInbox::AccountStandingEnum::GOOD },
    { "warning", ToolsOzoneInbox::AccountStandingEnum::WARNING },
    { "atRisk", ToolsOzoneInbox::AccountStandingEnum::AT_RISK }
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

EnforcementView::SharedPtr EnforcementView::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<EnforcementView>();
    XJsonObject xjson(json);
    view->mState = xjson.getRequiredEnum<EnforcementState>("state");
    view->mScope = xjson.getOptionalEnum<ReportScope>("scope");
    view->mExpiresAt = xjson.getOptionalDateTime("expiresAt");
    view->mLabels = xjson.getOptionalStringList("labels");
    return view;
}

AppealView::SharedPtr AppealView::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<AppealView>();
    XJsonObject xjson(json);
    view->mState = xjson.getRequiredEnum<AppealState>("state");
    view->mAppealedAt = xjson.getOptionalDateTime("appealedAt");
    view->mResolvedAt = xjson.getOptionalDateTime("resolvedAt");
    view->mOutcome = xjson.getOptionalEnum<AppealOutcome>("outcome");
    view->mAppealableUntil = xjson.getOptionalDateTime("appealableUntil");
    return view;
}

PolicyView::SharedPtr PolicyView::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<PolicyView>();
    XJsonObject xjson(json);
    view->mKey = xjson.getRequiredString("key");
    view->mDisplayName = xjson.getRequiredString("displayName");
    view->mLink = xjson.getRequiredString("link");
    return view;
}

ActionView::SharedPtr ActionView::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<ActionView>();
    XJsonObject xjson(json);
    view->mId = xjson.getRequiredInt("id");
    view->mType = xjson.getRequiredString("type");
    view->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    view->mReversedAt = xjson.getOptionalDateTime("reversedAt");
    view->mExpiresAt = xjson.getOptionalDateTime("expiresAt");
    view->mLabels = xjson.getOptionalStringList("labels");
    view->mPolicies = xjson.getOptionalVector<PolicyView>("policies");
    return view;
}

SubjectView::SharedPtr SubjectView::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<SubjectView>();
    XJsonObject xjson(json);
    view->mSrc = xjson.getRequiredString("src");
    view->mSubject = xjson.getRequiredVariant<ComATProtoAdmin::RepoRef,
                                              ComATProtoRepo::StrongRef,
                                              UnknownVariant>("subject");
    view->mEnforcement = xjson.getRequiredObject<EnforcementView>("enforcement");
    view->mAppeal = xjson.getOptionalObject<AppealView>("appeal");
    view->mAvailableActions = xjson.getOptionalEnumVector<SubjectAction>("availableActions");
    view->mLatestAction = xjson.getOptionalObject<ActionView>("latestAction");
    view->mActionCount = xjson.getOptionalInt("actionCount");
    view->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    view->mUpdatedAt = xjson.getRequiredDateTime("updatedAt");
    view->mIsRead = xjson.getRequiredBool("isRead");
    return view;
}

AccountPreview::SharedPtr AccountPreview::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<AccountPreview>();
    XJsonObject xjson(json);
    view->mHandle = xjson.getOptionalString("handle");
    view->mProfile = xjson.getOptionalVariant<AppBskyActor::Profile, UnknownVariant>("profile");
    return view;
}

ReportsSummary::SharedPtr ReportsSummary::fromJson(const QJsonObject& json)
{
    auto summary = std::make_shared<ReportsSummary>();
    XJsonObject xjson(json);
    summary->mReasonTypes = xjson.getRequiredStringList("reasonTypes");
    summary->mFirstReportedOn = xjson.getRequiredDateTime("firstReportedOn");
    summary->mLastReportedOn = xjson.getRequiredDateTime("lastReportedOn");
    return summary;
}

SubjectViewDetail::SharedPtr SubjectViewDetail::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<SubjectViewDetail>();
    XJsonObject xjson(json);
    view->mSrc = xjson.getRequiredString("src");
    view->mIsRead = xjson.getRequiredBool("isRead");
    view->mSubject = xjson.getRequiredVariant<ComATProtoAdmin::RepoRef,
                                              ComATProtoRepo::StrongRef,
                                              UnknownVariant>("subject");
    view->mRecord = xjson.getOptionalVariant<AppBskyFeed::Record::Post,
                                             AppBskyGraph::List,
                                             AppBskyFeed::Generator,
                                             AppBskyGraph::StarterPack,
                                             AppBskyActor::Status,
                                             AppBskyActor::Link,
                                             UnknownVariant>("record");
    view->mEnforcement = xjson.getRequiredObject<EnforcementView>("enforcement");
    view->mAppeal = xjson.getOptionalObject<AppealView>("appeal");
    view->mAvailableActions = xjson.getOptionalEnumVector<SubjectAction>("availableActions");
    view->mActions = xjson.getRequiredVector<ActionView>("actions");
    view->mCursor = xjson.getOptionalString("cursor");
    view->mReports = xjson.getOptionalObject<ReportsSummary>("reports");
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

GetReportOutput::SharedPtr GetReportOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetReportOutput>();
    XJsonObject xjson(json);
    output->mReport = xjson.getRequiredObject<ReportView>("report");
    output->mResolution = xjson.getOptionalObject<ResolutionView>("resolution");
    return output;
}

ListActionedSubjectsOutput::SharedPtr ListActionedSubjectsOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<ListActionedSubjectsOutput>();
    XJsonObject xjson(json);
    output->mCursor = xjson.getOptionalString("cursor");
    output->mSubjects = xjson.getRequiredVector<SubjectView>("subjects");
    return output;
}

QJsonObject AppealActionRef::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("id", mId);
    return json;
}

QJsonObject AppealLabelRef::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("val", mVal);
    return json;
}

QJsonObject AppealTakedownRef::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    return json;
}

GetAccountStatusOutput::SharedPtr GetAccountStatusOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetAccountStatusOutput>();
    XJsonObject xjson(json);
    output->mSrc = xjson.getRequiredString("src");
    output->mStanding = xjson.getRequiredEnum<AccountStanding>("standing");
    output->mUpdatedAt = xjson.getRequiredDateTime("updatedAt");
    return output;
}

}