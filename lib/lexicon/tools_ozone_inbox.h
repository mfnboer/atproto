// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
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

// tools.ozone.inbox.listReports#reportView
// tools.ozone.inbox.getReport#reportView
struct ReportView
{
    using SubjectType = std::variant<
        ComATProtoAdmin::RepoRef::SharedPtr,
        ComATProtoRepo::StrongRef::SharedPtr,
        ChatBskyConvo::MessageRef::SharedPtr,
        ChatBskyConvo::ConvoRef::SharedPtr>;

    QString mSrc; // DID of moderation service
    int mId = -1; // report ID
    bool mIsRead = false;
    QString mReasonType;
    std::optional<QString> mReason;
    std::optional<QString> mLastActionTaken; // only listReports
    std::optional<ReportScope> mScope; // only listReports
    SubjectType mSubject;
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

}