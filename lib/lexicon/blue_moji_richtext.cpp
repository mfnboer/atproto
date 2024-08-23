// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "blue_moji_richtext.h"
#include "../xjson.h"

namespace ATProto::BlueMojiRichtext {

QJsonObject Formats_v0::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    XJsonObject::insertOptionalJsonValue(json, "png_128", mPng128);
    XJsonObject::insertOptionalJsonValue(json, "webp_128", mWebp128);
    XJsonObject::insertOptionalJsonValue(json, "gif_128", mGif128);
    XJsonObject::insertBoolIfTrue(json, "apng_128", mApng128);
    XJsonObject::insertBoolIfTrue(json, "lottie", mLottie);
    return json;
}

Formats_v0::SharedPtr Formats_v0::fromJson(const QJsonObject& json)
{
    const XJsonObject xjson(json);
    auto formats = std::make_shared<Formats_v0>();
    formats->mPng128 = xjson.getOptionalString("png_128");
    formats->mWebp128 = xjson.getOptionalString("webp_128");
    formats->mGif128 = xjson.getOptionalString("gif_128");
    formats->mApng128 = xjson.getOptionalBool("apng_128", false);
    formats->mLottie = xjson.getOptionalBool("lottie", false);
    return formats;
}

QJsonObject FacetBlueMoji::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("did", mDid);
    json.insert("name", mName);
    XJsonObject::insertOptionalJsonValue(json, "alt", mAlt);
    XJsonObject::insertBoolIfTrue(json, "adultOnly", mAdultOnly);
    XJsonObject::insertVariant(json, "formats", mFormats);
    return json;
}

FacetBlueMoji::SharedPtr FacetBlueMoji::fromJson(const QJsonObject& json)
{
    const XJsonObject xjson(json);
    auto blueMoji = std::make_shared<FacetBlueMoji>();
    blueMoji->mDid = xjson.getRequiredString("did");
    blueMoji->mName = xjson.getRequiredString("name");
    blueMoji->mAlt = xjson.getOptionalString("alt");
    blueMoji->mAdultOnly = xjson.getOptionalBool("adultOnly", false);
    blueMoji->mFormats = xjson.getRequiredVariant<Formats_v0>("formats");
    return blueMoji;
}

}
