// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include "app_bsky_richtext.h"
#include "com_atproto_label.h"
#include "com_atproto_repo.h"
#include "lexicon.h"
#include <QJsonDocument>

namespace ATProto::AppBskyFeed {

// app.bsky.feed.defs#generatorViewerState
struct GeneratorViewerState
{
    std::optional<QString> mLike; // at-uri

    using Ptr = std::unique_ptr<GeneratorViewerState>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.feed.defs#generatorView
struct GeneratorView {
    QString mUri;
    QString mCid;
    QString mDid;
    AppBskyActor::ProfileView::Ptr mCreator; // required
    QString mDisplayName;
    std::optional<QString> mDescription; // max 300 graphemes, 3000 bytes
    AppBskyRichtext::FacetList mDescriptionFacets;
    std::optional<QString> mAvatar;
    int mLikeCount = 0;
    GeneratorViewerState::Ptr mViewer; // optional
    QDateTime mIndexedAt;

    using SharedPtr = std::shared_ptr<GeneratorView>;
    using Ptr = std::unique_ptr<GeneratorView>;
    static Ptr fromJson(const QJsonObject& json);
};
using GeneratorViewList = std::vector<GeneratorView::Ptr>;

}

namespace ATProto::AppBskyEmbed {

// app.bsky.embed.images#aspectRatio
struct AspectRatio
{
    int mWidth; // >=1
    int mHeight; // >=1

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<AspectRatio>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.embed.images#image
struct Image
{
    Blob::Ptr mImage; // max 1,000,000 bytes mime: image/*
    QString mAlt;
    AspectRatio::Ptr mAspectRatio; // optional

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Image>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.embed.images
struct Images
{
    std::vector<Image::Ptr> mImages; // max 4

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Images>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.embed.images#viewImage
struct ImagesViewImage
{
    QString mThumb;
    QString mFullSize;
    QString mAlt;
    AspectRatio::Ptr mAspectRatio; // optional

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

// app.bsky.embed.external#external
struct ExternalExternal
{
    QString mUri;
    QString mTitle;
    QString mDescription;
    Blob::Ptr mThumb; // optional: max 1,000,000 bytes mime: image/*

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<ExternalExternal>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.embed.external
struct External
{
    ExternalExternal::Ptr mExternal;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<External>;
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

// app.bsky.embed.record
struct Record
{
    ComATProtoRepo::StrongRef::Ptr mRecord;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Record>;
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
                 RecordViewBlocked::Ptr,
                 AppBskyFeed::GeneratorView::Ptr> mRecord;
    RecordType mRecordType;

    QString mUnsupportedType; // not part of the spec

    using Ptr = std::unique_ptr<RecordView>;
    static Ptr fromJson(const QJsonObject& json);
};

enum class EmbedType
{
    IMAGES,
    EXTERNAL,
    RECORD,
    RECORD_WITH_MEDIA,
    UNKNOWN
};

EmbedType stringToEmbedType(const QString& str);

// app.bsky.embed.recordWithMedia
struct RecordWithMedia
{
    Record::Ptr mRecord;
    std::variant<Images::Ptr, External::Ptr> mMedia;
    EmbedType mMediaType;
    QString mRawMediaType;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<RecordWithMedia>;
    static Ptr fromJson(const QJsonObject& json);
};

using EmbedUnion = std::variant<Images::Ptr,
                                External::Ptr,
                                Record::Ptr,
                                RecordWithMedia::Ptr>;

struct Embed
{
    EmbedUnion mEmbed;
    EmbedType mType;
    QString mRawType;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Embed>;
    static Ptr fromJson(const QJsonObject& json);
};

enum class EmbedViewType
{
    IMAGES_VIEW,
    EXTERNAL_VIEW,
    RECORD_VIEW,
    RECORD_WITH_MEDIA_VIEW,
    UNKNOWN
};

EmbedViewType stringToEmbedViewType(const QString& str);

// app.bsky.embed.recordWithMedia#view
struct RecordWithMediaView
{
    RecordView::Ptr mRecord;
    std::variant<ImagesView::Ptr, ExternalView::Ptr> mMedia;
    EmbedViewType mMediaType;
    QString mRawMediaType;

    using Ptr = std::unique_ptr<RecordWithMediaView>;
    static Ptr fromJson(const QJsonObject& json);
};

using EmbedViewUnion = std::variant<ImagesView::Ptr,
                                ExternalView::Ptr,
                                RecordView::Ptr,
                                RecordWithMediaView::Ptr>;

struct EmbedView
{
    EmbedViewUnion mEmbed;
    EmbedViewType mType;
    QString mRawType;

    using Ptr = std::unique_ptr<EmbedView>;
    static Ptr fromJson(const QJsonObject& json);
};

}

namespace ATProto::AppBskyFeed {

// app.bsky.feed.post#replyRef
struct PostReplyRef
{
    ComATProtoRepo::StrongRef::Ptr mRoot; // required
    ComATProtoRepo::StrongRef::Ptr mParent; // required

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<PostReplyRef>;
    static Ptr fromJson(const QJsonObject& json);
};

// Record types
namespace Record {

// app.bsky.feed.post
struct Post
{
    QString mText; // max 300 graphemes, 3000 bytes
    AppBskyRichtext::FacetList mFacets;
    PostReplyRef::Ptr mReply;
    ATProto::AppBskyEmbed::Embed::Ptr mEmbed; // optional
    // NOT IMPLEMENTED self labels
    // NOT IMPLEMENTED langs
    QDateTime mCreatedAt;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Post>;
    using SharedPtr = std::shared_ptr<Post>;
    static Ptr fromJson(const QJsonObject& json);
};

}}

namespace ATProto::AppBskyEmbed {

// app.bsky.embed.record#viewRecord
struct RecordViewRecord
{
    QString mUri;
    QString mCid;
    AppBskyActor::ProfileViewBasic::Ptr mAuthor; // required
    std::variant<AppBskyFeed::Record::Post::Ptr, AppBskyFeed::GeneratorView::Ptr> mValue;
    RecordType mValueType;
    QString mRawValueType;
    std::vector<ComATProtoLabel::Label::Ptr> mLabels;
    std::vector<EmbedView::Ptr> mEmbeds;
    QDateTime mIndexedAt;

    using Ptr = std::unique_ptr<RecordViewRecord>;
    static Ptr fromJson(const QJsonObject& json);
};

}
