// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "com_atproto_repo.h"
#include "../xjson.h"

namespace ATProto::ComATProtoRepo {

CommitMeta::SharedPtr CommitMeta::fromJson(const QJsonObject& json)
{
    auto commit = std::make_shared<CommitMeta>();
    const XJsonObject xjson(json);
    commit->mCid = xjson.getRequiredString("cid");
    commit->mRev = xjson.getRequiredString("rev");
    return commit;
}

QJsonObject StrongRef::toJson() const
{
    QJsonObject json;
    json.insert("$type", "com.atproto.repo.strongRef");
    json.insert("uri", mUri);
    json.insert("cid", mCid);
    return json;
}

StrongRef::SharedPtr StrongRef::fromJson(const QJsonObject& json)
{
    auto strongRef = std::make_shared<StrongRef>();
    const XJsonObject xjson(json);
    strongRef->mUri = xjson.getRequiredString("uri");
    strongRef->mCid = xjson.getRequiredString("cid");
    return strongRef;
}

UploadBlobOutput::SharedPtr UploadBlobOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<UploadBlobOutput>();
    const XJsonObject xjson(json);
    output->mBlob = xjson.getRequiredObject<Blob>("blob");
    return output;
}

Record::SharedPtr Record::fromJson(const QJsonObject& json)
{
    auto record = std::make_shared<Record>();
    const XJsonObject xjson(json);
    record->mUri = xjson.getRequiredString("uri");
    record->mCid = xjson.getOptionalString("cid");
    record->mValue = xjson.getRequiredJsonObject("value");
    return record;
}

ListRecordsOutput::SharedPtr ListRecordsOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<ListRecordsOutput>();
    const XJsonObject xjson(json);
    output->mCursor = xjson.getOptionalString("cursor");
    output->mRecords = xjson.getRequiredVector<Record>("records");
    return output;
}

QJsonObject ApplyWritesCreate::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("collection", mCollection);
    XJsonObject::insertOptionalJsonValue(json, "rkey", mRKey);
    json.insert("value", mValue);
    return json;
}

QJsonObject ApplyWritesUpdate::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("collection", mCollection);
    json.insert("rkey", mRKey);
    json.insert("value", mValue);
    return json;
}

QJsonObject ApplyWritesDelete::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("collection", mCollection);
    json.insert("rkey", mRKey);
    return json;
}

ApplyWritesCreateResult::SharedPtr ApplyWritesCreateResult::fromJson(const QJsonObject& json)
{
    auto result = std::make_shared<ApplyWritesCreateResult>();
    const XJsonObject xjson(json);
    result->mUri = xjson.getRequiredString("uri");
    result->mCid = xjson.getRequiredString("cid");
    result->mValidationStatus = xjson.getOptionalString("validationStatus");
    return result;
}

ApplyWritesUpdateResult::SharedPtr ApplyWritesUpdateResult::fromJson(const QJsonObject& json)
{
    auto result = std::make_shared<ApplyWritesUpdateResult>();
    const XJsonObject xjson(json);
    result->mUri = xjson.getRequiredString("uri");
    result->mCid = xjson.getRequiredString("cid");
    result->mValidationStatus = xjson.getOptionalString("validationStatus");
    return result;
}

ApplyWritesDeleteResult::SharedPtr ApplyWritesDeleteResult::fromJson(const QJsonObject&)
{
    auto result = std::make_shared<ApplyWritesDeleteResult>();
    return result;
}

ApplyWritesOutput::SharedPtr ApplyWritesOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<ApplyWritesOutput>();
    const XJsonObject xjson(json);
    output->mCommit = xjson.getOptionalObject<CommitMeta>("commit");

    output->mResults = xjson.getOptionalVariantList<
        ApplyWritesCreateResult,
        ApplyWritesUpdateResult,
        ApplyWritesDeleteResult>("results");

    return output;
}

}
