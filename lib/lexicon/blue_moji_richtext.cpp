// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "blue_moji_richtext.h"
#include "../xjson.h"

namespace ATProto::BlueMojiRichtext {

QJsonObject FacetBlueMoji::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("name", mName);
    json.insert("uri", mUri);
    XJsonObject::insertOptionalJsonValue(json, "alt", mAlt);
    return json;
}

FacetBlueMoji::SharedPtr FacetBlueMoji::fromJson(const QJsonObject& json)
{
    const XJsonObject xjson(json);
    auto blueMoji = std::make_shared<FacetBlueMoji>();
    blueMoji->mName = xjson.getRequiredString("name");
    blueMoji->mUri = xjson.getRequiredString("uri");
    blueMoji->mAlt = xjson.getOptionalString("alt");
    return blueMoji;
}

}
