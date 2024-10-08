// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "lexicon.h"
#include "app_bsky_embed.h"
#include "app_bsky_graph.h"
#include "app_bsky_labeler.h"
#include "../xjson.h"
#include <unordered_map>

namespace ATProto {

ATProtoError::SharedPtr ATProtoError::fromJson(const QJsonDocument& json)
{
    auto error = std::make_shared<ATProtoError>();
    const auto jsonObj = json.object();
    const XJsonObject xjson(jsonObj);
    error->mError = xjson.getRequiredString("error");
    const auto msg = xjson.getOptionalString("message");
    error->mMessage = msg.value_or(error->mError);
    return error;
}

Blob::SharedPtr Blob::fromJson(const QJsonObject& json)
{
    auto blob = std::make_shared<Blob>();
    const XJsonObject xjson(json);
    blob->mJson = json;
    
    const auto refJson = xjson.getOptionalJsonObject("ref");
    if (refJson)
    {
        const XJsonObject xRefJson(*refJson);
        blob->mRefLink = xRefJson.getRequiredString("$link");
        blob->mSize = xjson.getRequiredInt("size");
    }
    else
    {
        blob->mCid = xjson.getRequiredString("cid");
        qDebug() << "Deprecated legacy blob cid:" << blob->mCid;
    }

    blob->mMimeType = xjson.getRequiredString("mimeType");
    return blob;
}

QJsonObject Blob::toJson() const
{
    QJsonObject json(mJson);
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
        { "app.bsky.feed.defs#generatorView", RecordType::APP_BSKY_FEED_GENERATOR_VIEW },
        { "app.bsky.graph.defs#listView", RecordType::APP_BSKY_GRAPH_LIST_VIEW },
        { AppBskyGraph::StarterPackViewBasic::TYPE, RecordType::APP_BSKY_GRAPH_STARTER_PACK_VIEW_BASIC },
        { AppBskyLabeler::LabelerView::TYPE, RecordType::APP_BSKY_LABELER_VIEW },
        { AppBskyEmbed::RecordViewBlocked::TYPE, RecordType::APP_BSKY_EMBED_RECORD_VIEW_BLOCKED },
        { AppBskyEmbed::RecordViewNotFound::TYPE, RecordType::APP_BSKY_EMBED_RECORD_VIEW_NOT_FOUND },
        { AppBskyEmbed::RecordViewDetached::TYPE, RecordType::APP_BSKY_EMBED_RECORD_VIEW_DETACHED },
        { AppBskyEmbed::RecordViewRecord::TYPE, RecordType::APP_BSKY_EMBED_RECORD_VIEW_RECORD }
    };

    const auto it = recordMapping.find(str);
    if (it != recordMapping.end())
        return it->second;

    qWarning() << "Unknown record type:" << str;
    return RecordType::UNKNOWN;
};

QString recordTypeToString(RecordType recordType)
{
    static const std::unordered_map<RecordType, QString> mapping = {
        { RecordType::APP_BSKY_FEED_POST, "app.bsky.feed.post" },
        { RecordType::APP_BSKY_FEED_GENERATOR_VIEW, "app.bsky.feed.defs#generatorView" },
        { RecordType::APP_BSKY_GRAPH_LIST_VIEW, "app.bsky.graph.defs#listView" },
        { RecordType::APP_BSKY_GRAPH_STARTER_PACK_VIEW_BASIC, AppBskyGraph::StarterPackViewBasic::TYPE },
        { RecordType::APP_BSKY_LABELER_VIEW, AppBskyLabeler::LabelerView::TYPE },
        { RecordType::APP_BSKY_EMBED_RECORD_VIEW_BLOCKED, AppBskyEmbed::RecordViewBlocked::TYPE },
        { RecordType::APP_BSKY_EMBED_RECORD_VIEW_NOT_FOUND, AppBskyEmbed::RecordViewNotFound::TYPE },
        { RecordType::APP_BSKY_EMBED_RECORD_VIEW_DETACHED, AppBskyEmbed::RecordViewDetached::TYPE },
        { RecordType::APP_BSKY_EMBED_RECORD_VIEW_RECORD, AppBskyEmbed::RecordViewRecord::TYPE },
    };

    const auto it = mapping.find(recordType);
    if (it != mapping.end())
        return it->second;

    return {};
}

DidDocument::SharedPtr DidDocument::fromJson(const QJsonObject& json)
{
    const XJsonObject xjson(json);
    auto didDoc = std::make_shared<DidDocument>();
    const auto services = xjson.getOptionalArray("service");

    if (services)
    {
        for (const auto serviceRef : *services)
        {
            if (!serviceRef.isObject())
                continue;

            const auto serviceJson = serviceRef.toObject();
            const XJsonObject serviceXJson(serviceJson);
            const QString id = serviceXJson.getOptionalString("id", "");
            const QString type = serviceXJson.getOptionalString("type", "");

            if (type == "AtprotoPersonalDataServer" && id == "#atproto_pds") {
                didDoc->mATProtoPDS = serviceXJson.getOptionalString("serviceEndpoint");
                break;
            }
        }
    }

    didDoc->mJson = json;
    return didDoc;
}

QString createAvatarThumbUrl(const QString& avatarUrl)
{
    QString url = avatarUrl;
    return url.replace("/img/avatar/plain/", "/img/avatar_thumbnail/plain/");
}

}
