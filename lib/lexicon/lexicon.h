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
// The default constructor constructs a variant with the first alternative set
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

    // Internal stack errors
    SHARED_CONST(QString, PDS_NOT_FOUND, QStringLiteral("PdsNotFound"));
    SHARED_CONST(QString, XRPC_TIMEOUT, QStringLiteral("XrpcTimeout"));

    static bool isRecordNotFound(const QString& error)
    {
        // Sometimes INVALID_REQUEST is returned when a record does not exist.
        return error == RECORD_NOT_FOUND || error == INVALID_REQUEST;
    }

    static bool isListNotFound(const QString& error)
    {
        // Currently INVALID_REQUEST is returned when a list does not exist. But I have
        // seen errors getting changed before. Test for NOT_FOUND as a precaution.
        return error == INVALID_REQUEST || error == NOT_FOUND;
    }
};

// HTTP API (XRPC): error responses must contain json body with error and message fields.
struct ATProtoError
{
    QString mError;
    QString mMessage;

    using SharedPtr = std::shared_ptr<ATProtoError>;
    static SharedPtr fromJson(const QJsonDocument& json);
};

template <typename EnumType>
EnumType stringToEnum(const QString& str, const std::unordered_map<QString, EnumType>& mapping, EnumType unknownValue)
{
    const auto it = mapping.find(str);

    if (it != mapping.end())
        return it->second;

    qWarning() << "Uknown value:" << str << "for type:" << typeid(EnumType).name();
    return unknownValue;
}

template <typename EnumType>
QString enumToString(EnumType value, const std::unordered_map<EnumType, QString>& mapping, const QString& unknown = "")
{
    const auto it = mapping.find(value);
    Q_ASSERT(it != mapping.end() || !unknown.isEmpty());

    if (it != mapping.end())
        return it->second;

    qWarning() << "Cannot convert value to string:" << int(value) << "for type:" << typeid(EnumType).name();
    return unknown;
}

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

void setOptionalString(std::optional<QString>& field, const QString& value);

}
