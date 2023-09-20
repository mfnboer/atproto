// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "com_atproto_repo.h"
#include "../xjson.h"

namespace ATProto::ComATProtoRepo {

QJsonObject StrongRef::toJson() const
{
    QJsonObject json;
    json.insert("uri", mUri);
    json.insert("cid", mCid);
    return json;
}

StrongRef::Ptr StrongRef::fromJson(const QJsonObject& json)
{
    auto strongRef = std::make_unique<StrongRef>();
    XJsonObject xjson(json);
    strongRef->mUri = xjson.getRequiredString("uri");
    strongRef->mCid = xjson.getRequiredString("cid");
    return strongRef;
}

UploadBlobOutput::Ptr UploadBlobOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_unique<UploadBlobOutput>();
    XJsonObject xjson(json);
    const auto blobJson = xjson.getRequiredObject("blob");
    output->mBlob = Blob::fromJson(blobJson);
    return output;
}

Record::Ptr Record::fromJson(const QJsonObject& json)
{
    auto record = std::make_unique<Record>();
    XJsonObject xjson(json);
    record->mUri = xjson.getRequiredString("uri");
    record->mCid = xjson.getOptionalString("cid");
    record->mValue = xjson.getRequiredObject("value");
    return record;
}

}
