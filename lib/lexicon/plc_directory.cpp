// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "plc_directory.h"
#include "../xjson.h"

namespace ATProto {

PlcError::Ptr PlcError::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto error = std::make_unique<PlcError>();
    error->mMessage = xjson.getOptionalString("message");
    return error;
}

PlcAuditLogEntry::Ptr PlcAuditLogEntry::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto entry = std::make_unique<PlcAuditLogEntry>();
    entry->mDid = xjson.getRequiredString("did");
    entry->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return entry;
}

PlcAuditLog::Ptr PlcAuditLog::fromJson(const QJsonDocument& json)
{
    if (!json.isArray())
        throw InvalidJsonException("PLC Audit Log must be an array");

    const QJsonArray jsonArray = json.array();
    auto log = std::make_unique<PlcAuditLog>();

    for (const auto& jsonElem : jsonArray)
    {
        auto entry = PlcAuditLogEntry::fromJson(jsonElem.toObject());
        log->mEntries.push_back(std::move(entry));
    }

    return log;
}

}
