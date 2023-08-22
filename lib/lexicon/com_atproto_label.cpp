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
    XJsonObject root(json);
    const auto labelsArray = root.getOptionalArray("labels");
    if (labelsArray)
    {
        for (const auto& labelJson : *labelsArray)
        {
            if (!labelJson.isObject())
            {
                qWarning() << "Invalid label:" << json;
                throw InvalidJsonException("Invalid label");
            }

            auto label = Label::fromJson(labelJson.toObject());
            labels.push_back(std::move(label));
        }
    }
}


}
