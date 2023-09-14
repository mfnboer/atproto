// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "lexicon.h"
#include "../xjson.h"
#include <unordered_map>

namespace ATProto {

ATProtoError::Ptr ATProtoError::fromJson(const QJsonDocument& json)
{
    auto error = std::make_unique<ATProtoError>();
    const auto jsonObj = json.object();
    const XJsonObject xjson(jsonObj);
    error->mError = xjson.getRequiredString("error");
    error->mMessage = xjson.getRequiredString("message");
    return error;
}

Blob::Ptr Blob::fromJson(const QJsonObject& json)
{
    auto blob = std::make_unique<Blob>();
    const XJsonObject xjson(json);
    const auto refJson = xjson.getRequiredObject("ref");
    const XJsonObject xRefJson(refJson);
    blob->mRefLink = xRefJson.getRequiredString("$link");
    blob->mMimeType = xjson.getRequiredString("mimeType");
    blob->mSize = xjson.getRequiredInt("size");
    return blob;
}

QJsonObject Blob::toJson() const
{
    QJsonObject json;
    json.insert("$type", "blob");
    QJsonObject refJson;
    refJson.insert("$link", mRefLink);
    json.insert("ref", refJson);
    json.insert("mimeType", mMimeType);
    json.insert("size", mSize);
    return json;
}

RecordType stringToRecordType(const QString& str)
{
    static const std::unordered_map<QString, RecordType> recordMapping = {
        { "app.bsky.feed.post", RecordType::APP_BSKY_FEED_POST },
        { "app.bsky.embed.record#viewBlocked", RecordType::APP_BSKY_EMBED_RECORD_VIEW_BLOCKED },
        { "app.bsky.embed.record#viewNotFound", RecordType::APP_BSKY_EMBED_RECORD_VIEW_NOT_FOUND },
        { "app.bsky.embed.record#viewRecord", RecordType::APP_BSKY_EMBED_RECORD_VIEW_RECORD }
    };

    const auto it = recordMapping.find(str);
    if (it != recordMapping.end())
        return it->second;

    qWarning() << "Unknown record type:" << str;
    return RecordType::UNKNOWN;
};

}
