// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>

namespace ATProto {

// HTTP API (XRPC): error responses must contain json body with error and message fields.
struct ATProtoError
{
    QString mError;
    QString mMessage;

    using Ptr = std::unique_ptr<ATProtoError>;
    static Ptr fromJson(const QJsonDocument& json);
};

struct Blob {
    QString mRefLink;
    QString mMimeType;
    int mSize;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Blob>;
    static Ptr fromJson(const QJsonObject& json);
};

enum class RecordType
{
    APP_BSKY_EMBED_RECORD_VIEW_NOT_FOUND,
    APP_BSKY_EMBED_RECORD_VIEW_BLOCKED,
    APP_BSKY_EMBED_RECORD_VIEW_RECORD,

    APP_BSKY_FEED_POST,

    UNKNOWN
};

RecordType stringToRecordType(const QString& str);

}
