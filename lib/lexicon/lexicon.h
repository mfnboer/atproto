// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "qml_utils.h"
#include <QException>
#include <QJsonDocument>
#include <QJsonObject>
#include <QtQmlIntegration>

namespace ATProto {

// Variant types in the lexicon are like this variant<T:Ptr, U:Ptr, ...>
// The default constructor construcst a variant with the first alternative set
// to its default value, i.e. nullptr
template<typename Variant>
bool isNullVariant(const Variant& variant)
{
    auto* value = std::get_if<0>(&variant);
    return value && !*value;
}

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

struct ATProtoErrorMsg : public QObject
{
    Q_OBJECT
    QML_UNCREATABLE("Class only exposes constants to QML.")
    QML_ELEMENT
    QML_SINGLETON

public:
    SHARED_CONST(QString, ALREADY_EXISTS, QStringLiteral("already_exists"));
    SHARED_CONST(QString, AUTH_FACTOR_TOKEN_REQUIRED, QStringLiteral("AuthFactorTokenRequired"));
    SHARED_CONST(QString, BLOCKED_ACTOR, QStringLiteral("BlockedActor"));
    SHARED_CONST(QString, EXPIRED_TOKEN, QStringLiteral("ExpiredToken"));
    SHARED_CONST(QString, INVALID_REQUEST, QStringLiteral("InvalidRequest"));
    SHARED_CONST(QString, INVALID_TOKEN, QStringLiteral("InvalidToken"));
    SHARED_CONST(QString, NOT_FOUND, QStringLiteral("NotFound"));
    SHARED_CONST(QString, RECORD_NOT_FOUND, QStringLiteral("RecordNotFound"));

    // Internal stack error
    SHARED_CONST(QString, XRPC_TIMEOUT, QStringLiteral("XrpcTimeout"));
};

// HTTP API (XRPC): error responses must contain json body with error and message fields.
struct ATProtoError
{
    QString mError;
    QString mMessage;

    using SharedPtr = std::shared_ptr<ATProtoError>;
    static SharedPtr fromJson(const QJsonDocument& json);
};

struct Blob {
    QString mRefLink; // may not be present in old-style blobs, instead cid is present
    QString mMimeType;
    int mSize;
    QString mCid; // deprecated but still in use
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Blob>;
    static SharedPtr fromJson(const QJsonObject& json);
};

enum class RecordType
{
    APP_BSKY_EMBED_RECORD_VIEW_NOT_FOUND,
    APP_BSKY_EMBED_RECORD_VIEW_BLOCKED,
    APP_BSKY_EMBED_RECORD_VIEW_DETACHED,
    APP_BSKY_EMBED_RECORD_VIEW_RECORD,

    APP_BSKY_FEED_POST,
    APP_BSKY_FEED_GENERATOR_VIEW,
    APP_BSKY_GRAPH_LIST_VIEW,
    APP_BSKY_GRAPH_STARTER_PACK_VIEW_BASIC,
    APP_BSKY_LABELER_VIEW,

    UNKNOWN
};

RecordType stringToRecordType(const QString& str);
QString recordTypeToString(RecordType recordType);

struct DidDocument {
    QString mId;
    std::optional<QString> mATProtoPDS;
    QJsonObject mJson;

    using SharedPtr = std::shared_ptr<DidDocument>;
    static SharedPtr fromJson(const QJsonObject& json);
};

QString createAvatarThumbUrl(const QString& avatarUrl);

}
