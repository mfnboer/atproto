// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "lexicon.h"
#include "app_bsky_embed.h"
#include "app_bsky_graph.h"
#include "app_bsky_labeler.h"
#include "../xjson.h"
#include <unordered_map>

namespace ATProto {

QJsonObject UnknownVariant::toJson() const
{
    return mJson;
}

UnknownVariant::SharedPtr UnknownVariant::fromJson(const QJsonObject& json)
{
    auto unknown = std::make_shared<UnknownVariant>();
    XJsonObject xjson(json);
    unknown->mType = xjson.getRequiredString("$type");
    unknown->mJson = json;
    return unknown;
}

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

            // Examples of valid id values:
            // "id": "#atproto_pds"
            // "id": "did:plc:ewvi7nxzyoun6zhxrhs64oiz#atproto_pds"
            if (type == "AtprotoPersonalDataServer" && id.endsWith("#atproto_pds")) {
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

void setOptionalString(std::optional<QString>& field, const QString& value)
{
    if (value.isEmpty())
        field = {};
    else
        field = value;
}

}
