// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>

namespace ATProto {

struct PlcError
{
    std::optional<QString> mMessage;

    using SharedPtr = std::shared_ptr<PlcError>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct PlcAuditLogEntry
{
    QString mDid;
    QDateTime mCreatedAt;

    using SharedPtr = std::shared_ptr<PlcAuditLogEntry>;
    static SharedPtr fromJson(const QJsonObject& json);
};

using PlcAutitLogEntryList = std::vector<PlcAuditLogEntry::SharedPtr>;

struct PlcAuditLog
{
    PlcAutitLogEntryList mEntries;

    using SharedPtr = std::shared_ptr<PlcAuditLog>;
    static SharedPtr fromJson(const QJsonDocument& json);
};

}
