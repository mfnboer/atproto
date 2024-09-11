// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_embed.h"
#include "../xjson.h"
#include <unordered_map>

namespace ATProto::AppBskyEmbed {

QJsonObject AspectRatio::toJson() const
{
    QJsonObject json;
    json.insert("width", mWidth);
    json.insert("height", mHeight);
    return json;
}

AspectRatio::SharedPtr AspectRatio::fromJson(const QJsonObject& json)
{
    auto aspectRatio = std::make_shared<AspectRatio>();
    const XJsonObject xjson(json);
    aspectRatio->mWidth = xjson.getRequiredInt("width");
    aspectRatio->mHeight = xjson.getRequiredInt("height");
    return aspectRatio;
}

QJsonObject Image::toJson() const
{
    QJsonObject json;
    QJsonObject blobJson = mImage->toJson();
    json.insert("image", blobJson);
    json.insert("alt", mAlt);

    if (mAspectRatio)
    {
        QJsonObject ratioJson = mAspectRatio->toJson();
        json.insert("aspectRatio", ratioJson);
    }
    return json;
}

Image::SharedPtr Image::fromJson(const QJsonObject& json)
{
    auto image = std::make_shared<Image>();
    const XJsonObject xjson(json);
    image->mImage = xjson.getRequiredObject<Blob>("image");
    image->mAlt = xjson.getRequiredString("alt");
    image->mAspectRatio = xjson.getOptionalObject<AspectRatio>("aspectRatio");
    return image;
}

QJsonObject Images::toJson() const
{
    QJsonObject json;
    json.insert("$type", "app.bsky.embed.images");

    QJsonArray jsonArray;
    for (const auto& image : mImages)
    {
        QJsonObject imgJson = image->toJson();
        jsonArray.append(imgJson);
    }

    json.insert("images", jsonArray);
    return json;
}

Images::SharedPtr Images::fromJson(const QJsonObject& json)
{
    auto images = std::make_shared<Images>();
    const XJsonObject xjson(json);
    images->mImages = xjson.getRequiredVector<Image>("images");
    return images;
}

ImagesViewImage::SharedPtr ImagesViewImage::fromJson(const QJsonObject& json)
{
    auto viewImage = std::make_shared<ImagesViewImage>();
    const XJsonObject xjson(json);
    viewImage->mThumb = xjson.getRequiredString("thumb");
    viewImage->mFullSize = xjson.getRequiredString("fullsize");
    viewImage->mAlt = xjson.getRequiredString("alt");
    viewImage->mAspectRatio = xjson.getOptionalObject<AspectRatio>("aspectRatio");
    viewImage->mJson = json;
    return viewImage;
}

ImagesView::SharedPtr ImagesView::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<ImagesView>();
    const XJsonObject xjson(json);
    view->mImages = xjson.getRequiredVector<ImagesViewImage>("images");
    return view;
}

QJsonObject VideoCaption::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("lang", mLang);
    json.insert("file", mFile->toJson());
    return json;
}

VideoCaption::SharedPtr VideoCaption::fromJson(const QJsonObject& json)
{
    auto caption = std::make_unique<VideoCaption>();
    const XJsonObject xjson(json);
    caption->mLang = xjson.getRequiredString("lang");
    caption->mFile = xjson.getRequiredObject<Blob>("file");
    return caption;
}

QJsonObject Video::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("video", mVideo->toJson());
    XJsonObject::insertOptionalArray<VideoCaption>(json, "captions", mCaptions);
    XJsonObject::insertOptionalJsonValue(json, "alt", mAlt);
    XJsonObject::insertOptionalJsonObject<AspectRatio>(json, "aspectRatio", mAspectRatio);
    return json;
}

Video::SharedPtr Video::fromJson(const QJsonObject& json)
{
    auto video = std::make_unique<Video>();
    const XJsonObject xjson(json);
    video->mVideo = xjson.getRequiredObject<Blob>("video");
    video->mCaptions = xjson.getOptionalVector<VideoCaption>("captions");
    video->mAlt = xjson.getOptionalString("alt");
    video->mAspectRatio = xjson.getOptionalObject<AspectRatio>("aspectRatio");
    return video;
}

VideoView::SharedPtr VideoView::fromJson(const QJsonObject& json)
{
    auto view = std::make_unique<VideoView>();
    const XJsonObject xjson(json);
    view->mCid = xjson.getRequiredString("cid");
    view->mPlaylist = xjson.getRequiredString("playlist");
    view->mThumbnail = xjson.getOptionalString("thumbnail");
    view->mAlt = xjson.getOptionalString("alt");
    view->mAspectRatio = xjson.getOptionalObject<AspectRatio>("aspectRatio");
    view->mJson = json;
    return view;
}

QJsonObject ExternalExternal::toJson() const
{
    QJsonObject json;
    json.insert("uri", mUri);
    json.insert("title", mTitle);
    json.insert("description", mDescription);

    if (mThumb)
        json.insert("thumb", mThumb->toJson());

    return json;
}

ExternalExternal::SharedPtr ExternalExternal::fromJson(const QJsonObject& json)
{
    auto external = std::make_shared<ExternalExternal>();
    const XJsonObject xjson(json);
    external->mUri = xjson.getRequiredString("uri");
    external->mTitle = xjson.getRequiredString("title");
    external->mDescription = xjson.getRequiredString("description");
    external->mThumb = xjson.getOptionalObject<Blob>("thumb");
    return external;
}

QJsonObject External::toJson() const
{
    QJsonObject json;
    json.insert("$type", "app.bsky.embed.external");
    json.insert("external", mExternal->toJson());
    return json;
}

External::SharedPtr External::fromJson(const QJsonObject& json)
{
    auto external = std::make_shared<External>();
    const XJsonObject xjson(json);
    external->mExternal = xjson.getRequiredObject<ExternalExternal>("external");
    return external;
}

ExternalViewExternal::SharedPtr ExternalViewExternal::fromJson(const QJsonObject& json)
{
    auto viewExternal = std::make_shared<ExternalViewExternal>();
    const XJsonObject xjson(json);
    viewExternal->mUri = xjson.getRequiredString("uri");
    viewExternal->mTitle = xjson.getRequiredString("title");
    viewExternal->mDescription = xjson.getRequiredString("description");
    viewExternal->mThumb = xjson.getOptionalString("thumb");
    return viewExternal;
}

ExternalView::SharedPtr ExternalView::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<ExternalView>();
    XJsonObject xjson(json);
    view->mExternal = xjson.getRequiredObject<ExternalViewExternal>("external");
    return view;
}

QJsonObject Record::toJson() const
{
    QJsonObject json;
    json.insert("$type", "app.bsky.embed.record");
    json.insert("record", mRecord->toJson());
    return json;
}

Record::SharedPtr Record::fromJson(const QJsonObject& json)
{
    auto record = std::make_shared<Record>();
    XJsonObject xjson(json);
    record->mRecord = xjson.getRequiredObject<ComATProtoRepo::StrongRef>("record");
    return record;
}

RecordViewNotFound::SharedPtr RecordViewNotFound::fromJson(const QJsonObject& json)
{
    auto viewNotFound = std::make_shared<RecordViewNotFound>();
    const XJsonObject xjson(json);
    viewNotFound->mUri = xjson.getRequiredString("uri");
    return viewNotFound;
}

RecordViewBlocked::SharedPtr RecordViewBlocked::fromJson(const QJsonObject& json)
{
    auto viewBlocked = std::make_shared<RecordViewBlocked>();
    const XJsonObject xjson(json);
    viewBlocked->mUri = xjson.getRequiredString("uri");
    return viewBlocked;
}

RecordViewDetached::SharedPtr RecordViewDetached::fromJson(const QJsonObject& json)
{
    auto viewDetached = std::make_shared<RecordViewDetached>();
    const XJsonObject xjson(json);
    viewDetached->mUri = xjson.getRequiredString("uri");
    return viewDetached;
}

RecordView::SharedPtr RecordView::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<RecordView>();
    const XJsonObject xjson(json);
    const auto recordJson = xjson.getRequiredJsonObject("record");
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
    case RecordType::APP_BSKY_EMBED_RECORD_VIEW_DETACHED:
        view->mRecord = RecordViewDetached::fromJson(recordJson);
        break;
    case ATProto::RecordType::APP_BSKY_FEED_GENERATOR_VIEW:
        view->mRecord = AppBskyFeed::GeneratorView::fromJson(recordJson);
        break;
    case ATProto::RecordType::APP_BSKY_GRAPH_LIST_VIEW:
        view->mRecord = AppBskyGraph::ListView::fromJson(recordJson);
        break;
    case ATProto::RecordType::APP_BSKY_GRAPH_STARTER_PACK_VIEW_BASIC:
        view->mRecord = AppBskyGraph::StarterPackViewBasic::fromJson(recordJson);
        break;
    case ATProto::RecordType::APP_BSKY_LABELER_VIEW:
        view->mRecord = AppBskyLabeler::LabelerView::fromJson(recordJson);
        break;
    default:
        qWarning() << "Unsupported record type in app.bsky.embed.record#view:" << type << json;
        view->mUnsupportedType = type;
        break;
    }

    return view;
}

EmbedType stringToEmbedType(const QString& str)
{
    static const std::unordered_map<QString, EmbedType> mapping = {
        { "app.bsky.embed.images", EmbedType::IMAGES },
        { Video::TYPE, EmbedType::VIDEO },
        { "app.bsky.embed.external", EmbedType::EXTERNAL },
        { "app.bsky.embed.record", EmbedType::RECORD },
        { "app.bsky.embed.recordWithMedia", EmbedType::RECORD_WITH_MEDIA }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    return EmbedType::UNKNOWN;
}

QJsonObject RecordWithMedia::toJson() const
{
    QJsonObject json;
    json.insert("$type", "app.bsky.embed.recordWithMedia");
    json.insert("record", mRecord->toJson());

    switch (mMediaType)
    {
    case EmbedType::IMAGES:
        json.insert("media", std::get<Images::SharedPtr>(mMedia)->toJson());
        break;
    case EmbedType::VIDEO:
        json.insert("media", std::get<Video::SharedPtr>(mMedia)->toJson());
        break;
    case EmbedType::EXTERNAL:
        json.insert("media", std::get<External::SharedPtr>(mMedia)->toJson());
        break;
    case EmbedType::RECORD:
    case EmbedType::RECORD_WITH_MEDIA:
    case EmbedType::UNKNOWN:
        qWarning() << "Unsupported media type in app.bsky.embed.recordWithMedia:" << mRawMediaType;
        throw InvalidContent(mRawMediaType);
        break;
    }

    return json;
}

RecordWithMedia::SharedPtr RecordWithMedia::fromJson(const QJsonObject& json)
{
    auto recordMedia = std::make_shared<RecordWithMedia>();
    const XJsonObject xjson(json);
    recordMedia->mRecord = xjson.getRequiredObject<Record>("record");
    const auto mediaJson = xjson.getRequiredJsonObject("media");
    const XJsonObject mediaXJson(mediaJson);
    recordMedia->mRawMediaType = mediaXJson.getRequiredString("$type");
    recordMedia->mMediaType = stringToEmbedType(recordMedia->mRawMediaType);

    switch (recordMedia->mMediaType)
    {
    case EmbedType::IMAGES:
        recordMedia->mMedia = Images::fromJson(mediaJson);
        break;
    case EmbedType::VIDEO:
        recordMedia->mMedia = Video::fromJson(mediaJson);
        break;
    case EmbedType::EXTERNAL:
        recordMedia->mMedia = External::fromJson(mediaJson);
        break;
    case EmbedType::RECORD:
    case EmbedType::RECORD_WITH_MEDIA:
    case EmbedType::UNKNOWN:
        qWarning() << "Unsupported media type in app.bsky.embed.recordWithMedia:"
                   << recordMedia->mRawMediaType;
        break;
    }

    return recordMedia;
}

EmbedViewType stringToEmbedViewType(const QString& str)
{
    static const std::unordered_map<QString, EmbedViewType> mapping = {
        { "app.bsky.embed.images#view", EmbedViewType::IMAGES_VIEW },
        { VideoView::TYPE, EmbedViewType::VIDEO_VIEW },
        { "app.bsky.embed.external#view", EmbedViewType::EXTERNAL_VIEW },
        { "app.bsky.embed.record#view", EmbedViewType::RECORD_VIEW },
        { "app.bsky.embed.recordWithMedia#view", EmbedViewType::RECORD_WITH_MEDIA_VIEW }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    return EmbedViewType::UNKNOWN;
}

QJsonObject Embed::toJson() const
{
    switch (mType)
    {
    case EmbedType::IMAGES:
        return std::get<Images::SharedPtr>(mEmbed)->toJson();
    case EmbedType::VIDEO:
        return std::get<Video::SharedPtr>(mEmbed)->toJson();
    case EmbedType::EXTERNAL:
        return std::get<External::SharedPtr>(mEmbed)->toJson();
    case EmbedType::RECORD:
        return std::get<Record::SharedPtr>(mEmbed)->toJson();
    case EmbedType::RECORD_WITH_MEDIA:
        return std::get<RecordWithMedia::SharedPtr>(mEmbed)->toJson();
    case EmbedType::UNKNOWN:
        qWarning() << "Unknown embed type:" << mRawType;
        throw InvalidContent(mRawType);
        break;
    }

    return {};
}

Embed::SharedPtr Embed::fromJson(const QJsonObject& json)
{
    auto embed = std::make_shared<Embed>();
    const XJsonObject xjson(json);
    embed->mRawType = xjson.getRequiredString("$type");
    embed->mType = stringToEmbedType(embed->mRawType);

    switch(embed->mType)
    {
    case EmbedType::IMAGES:
        embed->mEmbed = Images::fromJson(json);
        break;
    case EmbedType::VIDEO:
        embed->mEmbed = Video::fromJson(json);
        break;
    case EmbedType::EXTERNAL:
        embed->mEmbed = External::fromJson(json);
        break;
    case EmbedType::RECORD:
        embed->mEmbed = Record::fromJson(json);
        break;
    case EmbedType::RECORD_WITH_MEDIA:
        embed->mEmbed = RecordWithMedia::fromJson(json);
        break;
    case EmbedType::UNKNOWN:
        qWarning() << "Unknown embed type:" << embed->mRawType;
        break;
    }

    return embed;
}

RecordWithMediaView::SharedPtr RecordWithMediaView::fromJson(const QJsonObject& json)
{
    auto recordMediaView = std::make_shared<RecordWithMediaView>();
    const XJsonObject xjson(json);
    recordMediaView->mRecord = xjson.getRequiredObject<RecordView>("record");
    const auto mediaJson = xjson.getRequiredJsonObject("media");
    const XJsonObject mediaXJson(mediaJson);
    recordMediaView->mRawMediaType = mediaXJson.getRequiredString("$type");
    recordMediaView->mMediaType = stringToEmbedViewType(recordMediaView->mRawMediaType);

    switch (recordMediaView->mMediaType)
    {
    case EmbedViewType::IMAGES_VIEW:
        recordMediaView->mMedia = ImagesView::fromJson(mediaJson);
        break;
    case EmbedViewType::VIDEO_VIEW:
        recordMediaView->mMedia = VideoView::fromJson(mediaJson);
        break;
    case EmbedViewType::EXTERNAL_VIEW:
        recordMediaView->mMedia = ExternalView::fromJson(mediaJson);
        break;
    case EmbedViewType::RECORD_VIEW:
    case EmbedViewType::RECORD_WITH_MEDIA_VIEW:
    case EmbedViewType::UNKNOWN:
        qWarning() << "Unsupported media type in app.bsky.embed.recordWithMedia#view:"
                   << recordMediaView->mRawMediaType;
        break;
    }

    return recordMediaView;
}

EmbedView::SharedPtr EmbedView::fromJson(const QJsonObject& json)
{
    auto embed = std::make_shared<EmbedView>();
    const XJsonObject xjson(json);
    embed->mRawType = xjson.getRequiredString("$type");
    embed->mType = stringToEmbedViewType(embed->mRawType);

    switch(embed->mType)
    {
    case EmbedViewType::IMAGES_VIEW:
        embed->mEmbed = ImagesView::fromJson(json);
        break;
    case EmbedViewType::VIDEO_VIEW:
        embed->mEmbed = VideoView::fromJson(json);
        break;
    case EmbedViewType::EXTERNAL_VIEW:
        embed->mEmbed = ExternalView::fromJson(json);
        break;
    case EmbedViewType::RECORD_VIEW:
        embed->mEmbed = RecordView::fromJson(json);
        break;
    case EmbedViewType::RECORD_WITH_MEDIA_VIEW:
        embed->mEmbed = RecordWithMediaView::fromJson(json);
        break;
    case EmbedViewType::UNKNOWN:
        qWarning() << "Unknown embed type:" << embed->mRawType;
        break;
    }

    return embed;
}

RecordViewRecord::SharedPtr RecordViewRecord::fromJson(const QJsonObject& json)
{
    auto viewRecord = std::make_shared<RecordViewRecord>();
    const XJsonObject xjson(json);
    viewRecord->mUri = xjson.getRequiredString("uri");
    viewRecord->mCid = xjson.getRequiredString("cid");
    viewRecord->mAuthor = xjson.getRequiredObject<AppBskyActor::ProfileViewBasic>("author");
    const auto valueJson = xjson.getRequiredJsonObject("value");
    const XJsonObject valueXJson(valueJson);
    viewRecord->mRawValueType = valueXJson.getRequiredString("$type");
    viewRecord->mValueType = stringToRecordType(viewRecord->mRawValueType);

    switch (viewRecord->mValueType)
    {
    case RecordType::APP_BSKY_FEED_POST:
        viewRecord->mValue = AppBskyFeed::Record::Post::fromJson(valueJson);
        break;
    case ATProto::RecordType::APP_BSKY_FEED_GENERATOR_VIEW:
        viewRecord->mValue = AppBskyFeed::GeneratorView::fromJson(valueJson);
        break;
    case ATProto::RecordType::APP_BSKY_GRAPH_LIST_VIEW:
        viewRecord->mValue = AppBskyGraph::ListView::fromJson(valueJson);
        break;
    case ATProto::RecordType::APP_BSKY_LABELER_VIEW:
        viewRecord->mValue = AppBskyLabeler::LabelerView::fromJson(valueJson);
        break;
    default:
        qWarning() << "Unsupported value type in app.bsky.embed.record#viewRecord:"
                   << viewRecord->mRawValueType;
        break;
    }

    ComATProtoLabel::getLabels(viewRecord->mLabels, json);
    viewRecord->mEmbeds = xjson.getOptionalVector<EmbedView>("embeds");
    viewRecord->mIndexedAt = xjson.getRequiredDateTime("indexedAt");
    return viewRecord;
}

}
