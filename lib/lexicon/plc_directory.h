// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>

namespace ATProto {

struct PlcError
{
    std::optional<QString> mMessage;

    using Ptr = std::unique_ptr<PlcError>;
    static Ptr fromJson(const QJsonObject& json);
};

struct PlcAuditLogEntry
{
    QString mDid;
    QDateTime mCreatedAt;

    using Ptr = std::unique_ptr<PlcAuditLogEntry>;
    static Ptr fromJson(const QJsonObject& json);
};

using PlcAutitLogEntryList = std::vector<PlcAuditLogEntry::Ptr>;

struct PlcAuditLog
{
    PlcAutitLogEntryList mEntries;

    using Ptr = std::unique_ptr<PlcAuditLog>;
    static Ptr fromJson(const QJsonDocument& json);
};

}
