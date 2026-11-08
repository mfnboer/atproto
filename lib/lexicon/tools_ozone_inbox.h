// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include "app_bsky_embed.h"
#include "app_bsky_feed.h"
#include "app_bsky_graph.h"
#include "chat_bsky_convo.h"
#include "com_atproto_admin.h"
#include "com_atproto_repo.h"
#include "qml_utils.h"
#include "string_enum.h"
#include <QJsonObject>
#include <QQmlEngine>

namespace ATProto::ToolsOzoneInbox {

struct OzoneInboxFilter : public QObject
{
    Q_OBJECT
    QML_UNCREATABLE("Class only exposes constants to QML.")
    QML_ELEMENT
    QML_SINGLETON

    SHARED_CONST(QString, ALL, QStringLiteral("all"));
    SHARED_CONST(QString, PENDING, QStringLiteral("pending"));
    SHARED_CONST(QString, RESOLVED, QStringLiteral("resolve"));
    SHARED_CONST(QString, UNREAD, QStringLiteral("unread"));
};

struct OzoneInboxSortField : public QObject
{
    Q_OBJECT
    QML_UNCREATABLE("Class only exposes constants to QML.")
    QML_ELEMENT
    QML_SINGLETON

    SHARED_CONST(QString, CREATED_AT, QStringLiteral("created_at"));
    SHARED_CONST(QString, UPDATED_AT, QStringLiteral("updated_at"));
};

struct OzoneInboxSortDirection : public QObject
{
    Q_OBJECT
    QML_UNCREATABLE("Class only exposes constants to QML.")
    QML_ELEMENT
    QML_SINGLETON

    SHARED_CONST(QString, CREATED_AT, QStringLiteral("asc"));
    SHARED_CONST(QString, UPDATED_AT, QStringLiteral("desc"));
};

struct OzoneInboxSection : public QObject
{
    Q_OBJECT
    QML_UNCREATABLE("Class only exposes constants to QML.")
    QML_ELEMENT
    QML_SINGLETON

    SHARED_CONST(QString, REPORTS, QStringLiteral("reports"));
    SHARED_CONST(QString, SUBJECTS, QStringLiteral("subjects"));
    SHARED_CONST(QString, ACCOUNT_STATUS, QStringLiteral("accountStatus"));
};

enum class ReportScopeEnum
{
    NETWORK,
    APP,
    LABEL_ONLY,
    UNKNOWN
};
using ReportScope = StringEnum<ReportScopeEnum, ReportScopeEnum::UNKNOWN>;

enum class ReportStatusEnum
{
    PENDING,
    RESOLVED,
    UNKNOWN
};
using ReportStatus = StringEnum<ReportStatusEnum, ReportStatusEnum::UNKNOWN>;

enum class ResolutionOutcomeEnum
{
    ACTION_TAKEN,
    NO_ACTION,
    OTHER,
    UNKNOWN
};
using ResolutionOutcome = StringEnum<ResolutionOutcomeEnum, ResolutionOutcomeEnum::UNKNOWN>;

// tools.ozone.inbox.listReports#reportView
// tools.ozone.inbox.getReport#reportView
struct ReportView
{
    using SubjectType = std::variant<
        ComATProtoAdmin::RepoRef::SharedPtr,
        ComATProtoRepo::StrongRef::SharedPtr,
        ChatBskyConvo::MessageRef::SharedPtr,
        ChatBskyConvo::ConvoRef::SharedPtr,
        UnknownVariant::SharedPtr>;

    using RecordType = std::variant<
        AppBskyFeed::Record::Post::SharedPtr,
        AppBskyGraph::List::SharedPtr,
        AppBskyFeed::Generator::SharedPtr,
        AppBskyGraph::StarterPack::SharedPtr,
        AppBskyActor::Status::SharedPtr,
        AppBskyActor::Link::SharedPtr,
        UnknownVariant::SharedPtr>;

    QString mSrc; // DID of moderation service
    int mId = -1; // report ID
    bool mIsRead = false; // only listReports
    QString mReasonType;
    std::optional<QString> mReason;
    std::optional<QString> mLastActionTaken; // only listReports (optional)
    std::optional<ReportScope> mScope; // only listReports (optional)
    SubjectType mSubject;
    std::optional<RecordType> mRecord; // only getReport (optional)
    ReportStatus mStatus;
    QDateTime mCreatedAt;
    QDateTime mUpdatedAt;

    using SharedPtr = std::shared_ptr<ReportView>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// tools.ozone.inbox.getReport#resolutionView
struct ResolutionView
{
    ResolutionOutcome mOutcome;
    std::optional<QString> mActionTaken;
    std::optional<ReportScope> mScope;
    QDateTime mResolvedAt;

    using SharedPtr = std::shared_ptr<ResolutionView>;
    static SharedPtr fromJson(const QJsonObject& json);
};

enum class EnforcementStateEnum
{
    NONE,
    LABELED,
    REMOVED,
    SUSPENDED,
    TAKENDOWN,
    UNKNOWN
};
using EnforcementState = StringEnum<EnforcementStateEnum, EnforcementStateEnum::UNKNOWN>;

// tools.ozone.inbox.defs#enformentView
struct EnforcementView
{
    EnforcementState mState;
    std::optional<ReportScope> mScope;
    std::optional<QDateTime> mExpiresAt;
    QStringList mLabels; // optional

    using SharedPtr = std::shared_ptr<EnforcementView>;
    static SharedPtr fromJson(const QJsonObject& json);
};

enum class AppealStateEnum
{
    NONE,
    PENDING,
    RESOLVED,
    SUPERSEDED,
    EXPIRED,
    UNKNOWN
};
using AppealState = StringEnum<AppealStateEnum, AppealStateEnum::UNKNOWN>;

enum class AppealOutcomeEnum
{
    APPROVED,
    DENIED,
    UNKNOWN
};
using AppealOutcome = StringEnum<AppealOutcomeEnum, AppealOutcomeEnum::UNKNOWN>;

enum class SubjectActionEnum
{
    APPEAL,
    UNKNOWN
};
using SubjectAction = StringEnum<SubjectActionEnum, SubjectActionEnum::UNKNOWN>;

// tools.ozone.inbox.defs#appealView
struct AppealView
{
    AppealState mState;
    std::optional<QDateTime> mAppealedAt;
    std::optional<QDateTime> mResolvedAt;
    std::optional<AppealOutcome> mOutcome;
    std::optional<QDateTime> mAppealableUntil;

    using SharedPtr = std::shared_ptr<AppealView>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// tools.ozone.inbox.defs#policyView
struct PolicyView
{
    QString mKey;
    QString mDisplayName;
    QString mLink; // uri

    using SharedPtr = std::shared_ptr<PolicyView>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// tools.ozone.inbox.defs#actionView
struct ActionView
{
    int mId;
    QString mType;
    std::optional<ReportScope> mScope;
    QDateTime mCreatedAt;
    std::optional<QDateTime> mReversedAt;
    std::optional<QDateTime> mExpiresAt;
    QStringList mLabels; // optional
    PolicyView::List mPolicies; // optional

    using SharedPtr = std::shared_ptr<ActionView>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// tools.ozone.inbox.defs#subjectView
struct SubjectView
{
    using SubjectType = std::variant<
        ComATProtoAdmin::RepoRef::SharedPtr,
        ComATProtoRepo::StrongRef::SharedPtr,
        UnknownVariant::SharedPtr>;

    QString mSrc; // DID of moderator
    SubjectType mSubject;
    EnforcementView::SharedPtr mEnforcement;
    AppealView::SharedPtr mAppeal; // optional
    std::vector<SubjectAction> mAvailableActions; // optional
    ActionView::SharedPtr mLatestAction; // optional
    std::optional<int> mActionCount;
    QDateTime mCreatedAt;
    QDateTime mUpdatedAt;
    bool mIsRead = true;

    using SharedPtr = std::shared_ptr<SubjectView>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// tools.ozone.inbox.defs#accountPreview
struct AccountPreview
{
    using ProfileType = std::variant<
        AppBskyActor::Profile::SharedPtr,
        UnknownVariant::SharedPtr>;

    std::optional<QString> mHandle;
    std::optional<ProfileType> mProfile;

    using SharedPtr = std::shared_ptr<AccountPreview>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct ReportsSummary
{
    QStringList mReasonTypes;
    QDateTime mFirstReportedOn;
    QDateTime mLastReportedOn;

    using SharedPtr = std::shared_ptr<ReportsSummary>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// tools.ozone.inbox.defs#subjectViewDetail
struct SubjectViewDetail
{
    using SubjectType = SubjectView::SubjectType;
    using RecordType = ReportView::RecordType;

    QString mSrc; // DID of moderator
    bool mIsRead = true;
    SubjectType mSubject;
    std::optional<RecordType> mRecord;
    AccountPreview::SharedPtr mAccount; // optional
    EnforcementView::SharedPtr mEnforcement;
    AppealView::SharedPtr mAppeal; // optional
    std::vector<SubjectAction> mAvailableActions; // optional
    ActionView::List mActions;
    std::optional<QString> mCursor;
    ReportsSummary::SharedPtr mReports; // optional
    QDateTime mCreatedAt;
    QDateTime mUpdatedAt;

    using SharedPtr = std::shared_ptr<SubjectViewDetail>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// tools.ozone.inbox.listReports#output
struct ListReportsOutput
{
    std::optional<QString> mCursor;
    ReportView::List mReports;

    using SharedPtr = std::shared_ptr<ListReportsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// tools.ozone.inbox.getReport#output
struct GetReportOutput
{
    ReportView::SharedPtr mReport;
    ResolutionView::SharedPtr mResolution; // optional

    using SharedPtr = std::shared_ptr<GetReportOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// tools.ozone.inbox.listActionedSubjects#output
struct ListActionedSubjectsOutput
{
    std::optional<QString> mCursor;
    SubjectView::List mSubjects;

    using SharedPtr = std::shared_ptr<ListActionedSubjectsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}