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

struct ReportsFilter : public QObject
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

struct ReportsSortField : public QObject
{
    Q_OBJECT
    QML_UNCREATABLE("Class only exposes constants to QML.")
    QML_ELEMENT
    QML_SINGLETON

    SHARED_CONST(QString, CREATED_AT, QStringLiteral("created_at"));
    SHARED_CONST(QString, UPDATED_AT, QStringLiteral("updated_at"));
};

struct ReportsSortDirection : public QObject
{
    Q_OBJECT
    QML_UNCREATABLE("Class only exposes constants to QML.")
    QML_ELEMENT
    QML_SINGLETON

    SHARED_CONST(QString, CREATED_AT, QStringLiteral("asc"));
    SHARED_CONST(QString, UPDATED_AT, QStringLiteral("desc"));
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

enum ResolutionOutcomeEnum
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

// tools.ozone.inbox.listReports#output
struct ListReportsOutput
{
    std::optional<QString> mCursor;
    ReportView::List mReports;

    using SharedPtr = std::shared_ptr<ListReportsOutput>;
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

// tools.ozone.inbox.getReport#output
struct GetReportOutput
{
    ReportView::SharedPtr mReport;
    ResolutionView::SharedPtr mResolution; // optional

    using SharedPtr = std::shared_ptr<GetReportOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}