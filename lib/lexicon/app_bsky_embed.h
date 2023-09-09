// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include "com_atproto_label.h"
#include "lexicon.h"
#include <QJsonDocument>

namespace ATProto::AppBskyEmbed {

// app.bsky.embed.images#viewImage
struct ImagesViewImage
{
    QString mThumb;
    QString mFullSize;
    QString mAlt;

    using Ptr = std::unique_ptr<ImagesViewImage>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.embed.images#view
struct ImagesView
{
    std::vector<ImagesViewImage::Ptr> mImages; // max length 4

    using Ptr = std::unique_ptr<ImagesView>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.embed.external#viewExternal
struct ExternalViewExternal
{
    QString mUri;
    QString mTitle;
    QString mDescription;
    std::optional<QString> mThumb;

    using Ptr = std::unique_ptr<ExternalViewExternal>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.embed.external#view
struct ExternalView
{
    ExternalViewExternal::Ptr mExternal; // required

    using Ptr = std::unique_ptr<ExternalView>;
    static Ptr fromJson(const QJsonObject& json);
};

struct RecordViewRecord;

// app.bsky.embed.record#viewNotFound
struct RecordViewNotFound
{
    QString mUri; // at-uri

    using Ptr = std::unique_ptr<RecordViewNotFound>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.embed.record#viewBlocked
struct RecordViewBlocked
{
    QString mUri; // at-uri

    using Ptr = std::unique_ptr<RecordViewBlocked>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.embed.record#view
struct RecordView
{
    std::variant<std::unique_ptr<RecordViewRecord>,
                 RecordViewNotFound::Ptr,
                 RecordViewBlocked::Ptr> mRecord;
    RecordType mRecordType;

    QString mUnsupportedType; // not part of the spec

    using Ptr = std::unique_ptr<RecordView>;
    static Ptr fromJson(const QJsonObject& json);
};

enum class EmbedType
{
    IMAGES_VIEW,
    EXTERNAL_VIEW,
    RECORD_VIEW,
    RECORD_WITH_MEDIA_VIEW,
    UNKNOWN
};

EmbedType stringToEmbedType(const QString& str);

// app.bsky.embed.recordWithMedia#view
struct RecordWithMediaView
{
    RecordView::Ptr mRecord;
    std::variant<ImagesView::Ptr, ExternalView::Ptr> mMedia;
    EmbedType mMediaType;

    using Ptr = std::unique_ptr<RecordWithMediaView>;
    static Ptr fromJson(const QJsonObject& json);
};

using EmbedUnion = std::variant<ImagesView::Ptr,
                                ExternalView::Ptr,
                                RecordView::Ptr,
                                RecordWithMediaView::Ptr>;

struct Embed
{
    EmbedUnion mEmbed;
    EmbedType mType;

    using Ptr = std::unique_ptr<Embed>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.embed.record#viewRecord
struct RecordViewRecord
{
    QString mUri;
    QString mCid;
    AppBskyActor::ProfileViewBasic::Ptr mAuthor; // required
    std::variant<AppBskyFeed::Record::Post::Ptr> mValue;
    RecordType mValueType;
    std::vector<ComATProtoLabel::Label::Ptr> mLabels;
    std::vector<Embed::Ptr> mEmbeds;
    QDateTime mIndexedAt;

    using Ptr = std::unique_ptr<RecordViewRecord>;
    static Ptr fromJson(const QJsonObject& json);
};

}
