// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "app_bsky_draft.h"
#include "../xjson.h"

namespace ATProto::AppBskyDraft {

QJsonObject DraftEmbedLocalRef::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", TYPE);
    json.insert("path", mPath);
    return json;
}

DraftEmbedLocalRef::SharedPtr DraftEmbedLocalRef::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto localRef = std::make_shared<DraftEmbedLocalRef>();
    localRef->mPath = xjson.getRequiredString("path");
    localRef->mJson = json;
    return localRef;
}

QJsonObject DraftEmbedCaption::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", TYPE);
    json.insert("lang", mLang);
    json.insert("content", mContent);
    return json;
}

DraftEmbedCaption::SharedPtr DraftEmbedCaption::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto caption = std::make_shared<DraftEmbedCaption>();
    caption->mLang = xjson.getRequiredString("lang");
    caption->mContent = xjson.getRequiredString("content");
    caption->mJson = json;
    return caption;
}

QJsonObject DraftEmbedImage::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", TYPE);
    json.insert("localRef", mLocalRef->toJson());
    XJsonObject::insertOptionalJsonValue(json, "alt", mAlt);
    return json;
}

DraftEmbedImage::SharedPtr DraftEmbedImage::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto image = std::make_shared<DraftEmbedImage>();
    image->mLocalRef = xjson.getRequiredObject<DraftEmbedLocalRef>("localRef");
    image->mAlt = xjson.getOptionalString("alt");
    image->mJson = json;
    return image;
}

QJsonObject DraftEmbedVideo::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", TYPE);
    json.insert("localRef", mLocalRef->toJson());
    XJsonObject::insertOptionalJsonValue(json, "alt", mAlt);
    XJsonObject::insertOptionalArray<DraftEmbedCaption>(json, "captions", mCaptions);
    return json;
}

DraftEmbedVideo::SharedPtr DraftEmbedVideo::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto video = std::make_shared<DraftEmbedVideo>();
    video->mLocalRef = xjson.getRequiredObject<DraftEmbedLocalRef>("localRef");
    video->mAlt = xjson.getOptionalString("alt");
    video->mCaptions = xjson.getOptionalVector<DraftEmbedCaption>("captions");
    video->mJson = json;
    return video;
}

QJsonObject DraftEmbedExternal::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", TYPE);
    json.insert("uri", mUri);
    return json;
}

DraftEmbedExternal::SharedPtr DraftEmbedExternal::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto external = std::make_shared<DraftEmbedExternal>();
    external->mUri = xjson.getRequiredString("uri");
    external->mJson = json;
    return external;
}

QJsonObject DraftEmbedRecord::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", TYPE);
    json.insert("record", mRecord->toJson());
    return json;
}

DraftEmbedRecord::SharedPtr DraftEmbedRecord::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto record = std::make_shared<DraftEmbedRecord>();
    record->mRecord = xjson.getRequiredObject<ComATProtoRepo::StrongRef>("record");
    record->mJson = json;
    return record;
}

QJsonObject DraftPost::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", TYPE);
    json.insert("text", mText);
    XJsonObject::insertOptionalJsonObject<ComATProtoLabel::SelfLabels>(json, "labels", mLabels);
    XJsonObject::insertOptionalArray<DraftEmbedImage>(json, "embedImages", mEmbedImages);
    XJsonObject::insertOptionalArray<DraftEmbedVideo>(json, "embedVideos", mEmbedVideos);
    XJsonObject::insertOptionalArray<DraftEmbedExternal>(json, "embedExternals", mEmbedExternals);
    XJsonObject::insertOptionalArray<DraftEmbedRecord>(json, "embedRecords", mEmbedRecords);
    return json;
}

DraftPost::SharedPtr DraftPost::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto post = std::make_shared<DraftPost>();
    post->mText = xjson.getRequiredString("text");
    post->mLabels = xjson.getOptionalObject<ComATProtoLabel::SelfLabels>("labels");
    post->mEmbedImages = xjson.getOptionalVector<DraftEmbedImage>("embedImages");
    post->mEmbedVideos = xjson.getOptionalVector<DraftEmbedVideo>("embedVideos");
    post->mEmbedExternals = xjson.getOptionalVector<DraftEmbedExternal>("embedExternals");
    post->mEmbedRecords = xjson.getOptionalVector<DraftEmbedRecord>("embedRecords");
    post->mJson = json;
    return post;
}

QJsonObject Draft::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", TYPE);
    json.insert("posts", XJsonObject::toJsonArray<DraftPost>(mPosts));
    XJsonObject::insertOptionalArray(json, "langs", mLangs);
    AppBskyFeed::PostgateEmbeddingRules::insertDisableEmbedding(json, "postgateEmbeddingRules", mDisableEmbedding);
    mThreadgateRules.insertRulesInto(json, "threadgateAllow");

    return json;
}

Draft::SharedPtr Draft::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto draft = std::make_shared<Draft>();
    draft->mDeviceId = xjson.getOptionalString("deviceId");
    draft->mDeviceName = xjson.getOptionalString("deviceName");
    draft->mPosts = xjson.getRequiredVector<DraftPost>("posts");
    draft->mLangs = xjson.getOptionalStringVector("langs");
    draft->mDisableEmbedding = AppBskyFeed::PostgateEmbeddingRules::getDisableEmbedding(json, "postgateEmbeddingRules");
    draft->mThreadgateRules = AppBskyFeed::ThreadgateRules::getRules(json, "threadgateAllow");
    draft->mJson = json;
    return draft;
}

QJsonObject DraftWithId::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", TYPE);
    json.insert("id", mId);
    json.insert("draft", mDraft->toJson());
    return json;
}

DraftWithId::SharedPtr DraftWithId::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto draft = std::make_shared<DraftWithId>();
    draft->mId = xjson.getRequiredString("id");
    draft->mDraft = xjson.getRequiredObject<Draft>("draft");
    draft->mJson = json;
    return draft;
}

DraftView::SharedPtr DraftView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_shared<DraftView>();
    view->mId = xjson.getRequiredString("id");
    view->mDraft = xjson.getRequiredObject<Draft>("draft");
    view->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    view->mUpdatedAt = xjson.getRequiredDateTime("updatedAt");
    return view;
}

GetDraftsOutput::SharedPtr GetDraftsOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<GetDraftsOutput>();
    output->mDrafts = xjson.getRequiredVector<DraftView>("drafts");
    return output;
}

CreateDraftOutput::SharedPtr CreateDraftOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<CreateDraftOutput>();
    output->mId = xjson.getRequiredString("id");
    return output;
}

}
