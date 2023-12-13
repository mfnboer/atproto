// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "com_atproto_label.h"
#include "../xjson.h"

namespace ATProto::ComATProtoLabel {

Label::Ptr Label::fromJson(const QJsonObject& json)
{
    auto label = std::make_unique<Label>();
    XJsonObject xjson(json);
    label->mSrc = xjson.getRequiredString("src");
    label->mUri = xjson.getRequiredString("uri");
    label->mCid = xjson.getOptionalString("cid");
    label->mVal = xjson.getRequiredString("val");
    label->mNeg = xjson.getOptionalBool("neg", false);
    label->mCreatedAt = xjson.getRequiredDateTime("cts");
    return label;
}

void getLabels(std::vector<Label::Ptr>& labels, const QJsonObject& json)
{
    XJsonObject xjson(json);
    labels = xjson.getOptionalVector<Label>("labels");
}

SelfLabel::Ptr SelfLabel::fromJson(const QJsonObject& json)
{
    auto label = std::make_unique<SelfLabel>();
    XJsonObject xjson(json);
    label->mJson = json;
    label->mVal = xjson.getRequiredString("val");
    return label;
}

QJsonObject SelfLabel::toJson() const
{
    QJsonObject json(mJson);
    json.insert("val", mVal);
    return json;
}

SelfLabels::Ptr SelfLabels::fromJson(const QJsonObject& json)
{
    auto labels = std::make_unique<SelfLabels>();
    XJsonObject xjson(json);
    labels->mJson = json;
    labels->mValues = xjson.getRequiredVector<SelfLabel>("values");
    return labels;
}

QJsonObject SelfLabels::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", "com.atproto.label.defs#selfLabels");
    json.insert("values", XJsonObject::toJsonArray<SelfLabel>(mValues));
    return json;
}

}
