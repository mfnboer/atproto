// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QException>
#include <QJsonDocument>
#include <QJsonObject>

namespace ATProto {

class InvalidContent : public QException
{
public:
    explicit InvalidContent(const QString& msg) : mMsg(msg) {}

    const QString& msg() const { return mMsg; }
    void raise() const override { throw *this; }
    InvalidContent *clone() const override { return new InvalidContent(*this); }

private:
    QString mMsg;
};

// HTTP API (XRPC): error responses must contain json body with error and message fields.
struct ATProtoError
{
    QString mError;
    QString mMessage;

    using Ptr = std::unique_ptr<ATProtoError>;
    static Ptr fromJson(const QJsonDocument& json);
};

struct Blob {
    QString mRefLink; // may not be present in old-style blobs, instead cid is present
    QString mMimeType;
    int mSize;
    QString mCid; // deprecated but still in use
    QJsonObject mJson;

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
    APP_BSKY_FEED_GENERATOR_VIEW,
    APP_BSKY_GRAPH_LIST_VIEW,

    UNKNOWN
};

RecordType stringToRecordType(const QString& str);

struct DidDocument {
    QString mId;
    std::optional<QString> mATProtoPDS;
    QJsonObject mJson;

    using Ptr = std::unique_ptr<DidDocument>;
    using SharedPtr = std::shared_ptr<DidDocument>;
    static Ptr fromJson(const QJsonObject& json);
};

}
