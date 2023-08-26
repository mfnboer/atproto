// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_embed.h"
#include "../xjson.h"
#include <unordered_map>

namespace ATProto::AppBskyEmbed {

ImagesViewImage::Ptr ImagesViewImage::fromJson(const QJsonObject& json)
{
    auto viewImage = std::make_unique<ImagesViewImage>();
    const XJsonObject xjson(json);
    viewImage->mThumb = xjson.getRequiredString("thumb");
    viewImage->mFullSize = xjson.getRequiredString("fullsize");
    viewImage->mAlt = xjson.getRequiredString("alt");
    return viewImage;
}

ImagesView::Ptr ImagesView::fromJson(const QJsonObject& json)
{
    auto view = std::make_unique<ImagesView>();
    const XJsonObject xjson(json);
    auto images = xjson.getRequiredArray("images");

    for (const auto& img : images)
    {
        if (!img.isObject())
            throw InvalidJsonException("Invalid viewImage");

        const auto imgJson = img.toObject();
        auto viewImage = ImagesViewImage::fromJson(imgJson);
        view->mImages.push_back(std::move(viewImage));
    }

    return view;
}

ExternalViewExternal::Ptr ExternalViewExternal::fromJson(const QJsonObject& json)
{
    auto viewExternal = std::make_unique<ExternalViewExternal>();
    const XJsonObject xjson(json);
    viewExternal->mUri = xjson.getRequiredString("uri");
    viewExternal->mTitle = xjson.getRequiredString("title");
    viewExternal->mDescription = xjson.getRequiredString("description");
    viewExternal->mThumb = xjson.getOptionalString("thumb");
    return viewExternal;
}

ExternalView::Ptr ExternalView::fromJson(const QJsonObject& json)
{
    auto view = std::make_unique<ExternalView>();
    XJsonObject xjson(json);
    const auto externalJson = xjson.getRequiredObject("external");
    view->mExternal = ExternalViewExternal::fromJson(externalJson);
    return view;
}

RecordViewNotFound::Ptr RecordViewNotFound::fromJson(const QJsonObject& json)
{
    auto viewNotFound = std::make_unique<RecordViewNotFound>();
    const XJsonObject xjson(json);
    viewNotFound->mUri = xjson.getRequiredString("uri");
    return viewNotFound;
}

RecordViewBlocked::Ptr RecordViewBlocked::fromJson(const QJsonObject& json)
{
    auto viewBlocked = std::make_unique<RecordViewBlocked>();
    const XJsonObject xjson(json);
    viewBlocked->mUri = xjson.getRequiredString("uri");
    return viewBlocked;
}

RecordView::Ptr RecordView::fromJson(const QJsonObject& json)
{
    auto view = std::make_unique<RecordView>();
    const XJsonObject xjson(json);
    const auto recordJson = xjson.getRequiredObject("record");
    const XJsonObject recordXJson(recordJson);
    const QString type = recordXJson.getRequiredString("$type");
    view->mRecordType = stringToRecordType(type);

    switch (view->mRecordType)
    {
    case RecordType::APP_BSKY_EMBED_RECORD_VIEW_RECORD:
        view->mRecord = RecordViewRecord::fromJson(recordJson);
        break;
    case RecordType::APP_BSKY_EMBED_RECORD_VIEW_NOT_FOUND:
        view->mRecord = RecordViewNotFound::fromJson(recordJson);
        break;
    case RecordType::APP_BSKY_EMBED_RECORD_VIEW_BLOCKED:
        view->mRecord = RecordViewBlocked::fromJson(recordJson);
        break;
    default:
        qWarning() << "Unsupported record type in app.bsky.embed.record#view:" << type << json;
        break;
    }

    return view;
}

EmbedType stringToEmbedType(const QString& str)
{
    static const std::unordered_map<QString, EmbedType> mapping = {
        { "app.bsky.embed.images#view", EmbedType::IMAGES_VIEW },
        { "app.bsky.embed.external#view", EmbedType::EXTERNAL_VIEW },
        { "app.bsky.embed.record#view", EmbedType::RECORD_VIEW },
        { "app.bsky.embed.recordWithMedia#view", EmbedType::RECORD_WITH_MEDIA_VIEW }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    return EmbedType::UNKNOWN;
}

RecordWithMediaView::Ptr RecordWithMediaView::fromJson(const QJsonObject& json)
{
    auto recordMediaView = std::make_unique<RecordWithMediaView>();
    const XJsonObject xjson(json);
    const auto recordJson = xjson.getRequiredObject("record");
    recordMediaView->mRecord = RecordView::fromJson(recordJson);
    const auto mediaJson = xjson.getRequiredObject("media");
    const XJsonObject mediaXJson(mediaJson);
    const QString type = mediaXJson.getRequiredString("$type");
    recordMediaView->mMediaType = stringToEmbedType(type);

    switch (recordMediaView->mMediaType)
    {
    case EmbedType::IMAGES_VIEW:
        recordMediaView->mMedia = ImagesView::fromJson(mediaJson);
        break;
    case EmbedType::EXTERNAL_VIEW:
        recordMediaView->mMedia = ExternalView::fromJson(mediaJson);
        break;
    default:
        qWarning() << "Unsupported media type in app.bsky.embed.recordWithMedia#view:" << type;
        break;
    }

    return recordMediaView;
}

Embed::Ptr Embed::fromJson(const QJsonObject& json)
{
    auto embed = std::make_unique<Embed>();
    const XJsonObject xjson(json);
    const QString type = xjson.getRequiredString("$type");
    embed->mType = stringToEmbedType(type);

    switch(embed->mType)
    {
    case EmbedType::IMAGES_VIEW:
        embed->mEmbed = ImagesView::fromJson(json);
        break;
    case EmbedType::EXTERNAL_VIEW:
        embed->mEmbed = ExternalView::fromJson(json);
        break;
    case EmbedType::RECORD_VIEW:
        embed->mEmbed = RecordView::fromJson(json);
        break;
    case EmbedType::RECORD_WITH_MEDIA_VIEW:
        embed->mEmbed = RecordWithMediaView::fromJson(json);
        break;
    default:
        qWarning() << "Unknow embed type:" << type;
        break;
    }

    return embed;
}

RecordViewRecord::Ptr RecordViewRecord::fromJson(const QJsonObject& json)
{
    auto viewRecord = std::make_unique<RecordViewRecord>();
    const XJsonObject xjson(json);
    viewRecord->mUri = xjson.getRequiredString("uri");
    viewRecord->mCid = xjson.getRequiredString("cid");
    const auto authorJson = xjson.getRequiredObject("author");
    viewRecord->mAuthor = AppBskyActor::ProfileViewBasic::fromJson(authorJson);
    const auto valueJson = xjson.getRequiredObject("value");
    const XJsonObject valueXJson(valueJson);
    const QString valueType = valueXJson.getRequiredString("$type");
    viewRecord->mValueType = stringToRecordType(valueType);

    switch (viewRecord->mValueType)
    {
    case RecordType::APP_BSKY_FEED_POST:
        viewRecord->mValue = AppBskyFeed::Record::Post::fromJson(valueJson);
        break;
    default:
        qWarning() << "Unsupported value type in app.bsky.embed.record#viewRecord:" << valueType;
        break;
    }

    ComATProtoLabel::getLabels(viewRecord->mLabels, json);

    const auto embeds = xjson.getOptionalArray("embeds");
    if (embeds)
    {
        for (const auto& embedJson : *embeds)
        {
            if (!embedJson.isObject())
                throw InvalidJsonException("Invalid embed in app.bsky.embed.record#viewRecord");

            auto embed = Embed::fromJson(embedJson.toObject());
            viewRecord->mEmbeds.push_back(std::move(embed));
        }
    }

    viewRecord->mIndexedAt = xjson.getRequiredDateTime("indexedAt");
    return viewRecord;
}

}
