// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include "app_bsky_graph.h"
#include "app_bsky_labeler.h"
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

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<GeneratorViewerState>;
    static SharedPtr fromJson(const QJsonObject& json);
};

enum class ContentMode
{
    UNSPECIFIED,
    VIDEO,
    UNKNOWN
};
ContentMode stringToContentMode(const QString& str);
QString contentModeToString(ContentMode mode, const QString& unknown);


// app.bsky.feed.defs#generatorView
struct GeneratorView {
    QString mUri;
    QString mCid;
    QString mDid;
    AppBskyActor::ProfileView::SharedPtr mCreator; // required
    QString mDisplayName;
    std::optional<QString> mDescription; // max 300 graphemes, 3000 bytes
    AppBskyRichtext::FacetList mDescriptionFacets;
    std::optional<QString> mAvatar;
    int mLikeCount = 0;
    bool mAcceptsInteractions = false;
    ComATProtoLabel::LabelList mLabels;
    GeneratorViewerState::SharedPtr mViewer; // optional
    std::optional<ContentMode> mContentMode;
    std::optional<QString> mRawContentMode;
    QDateTime mIndexedAt;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<GeneratorView>;
    static SharedPtr fromJson(const QJsonObject& json);
};
using GeneratorViewList = std::vector<GeneratorView::SharedPtr>;

}

namespace ATProto::AppBskyGraph {

// app.bsky.graph.defs#starterPackView
struct StarterPackView
{
    QString mUri; // at-uri
    QString mCid;
    std::variant<StarterPack::SharedPtr> mRecord;
    AppBskyActor::ProfileViewBasic::SharedPtr mCreator;
    ListViewBasic::SharedPtr mList; // optional
    ListItemViewList mListItemsSample;
    AppBskyFeed::GeneratorViewList mFeeds;
    int mJoinedWeekCount = 0;
    int mJoinedAllTimeCount = 0;
    ComATProtoLabel::LabelList mLabels;
    QDateTime mIndexedAt;

    using SharedPtr = std::shared_ptr<StarterPackView>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.graph.getStarterPack/output
struct GetStarterPackOutput
{
    StarterPackView::SharedPtr mStarterPack;

    using SharedPtr = std::shared_ptr<GetStarterPackOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}

namespace ATProto::AppBskyEmbed {

// app.bsky.embed.defs#aspectRatio
struct AspectRatio
{
    int mWidth; // >=1
    int mHeight; // >=1

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<AspectRatio>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.embed.images#image
struct Image
{
    Blob::SharedPtr mImage; // max 1,000,000 bytes mime: image/*
    QString mAlt;
    AspectRatio::SharedPtr mAspectRatio; // optional

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Image>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr int MAX_BYTES = 1'000'000;
};
using ImageList = std::vector<Image::SharedPtr>;

// app.bsky.embed.images
struct Images
{
    ImageList mImages; // max 4

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Images>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr int MAX_IMAGES = 4;
};

// app.bsky.embed.images#viewImage
struct ImagesViewImage
{
    QString mThumb;
    QString mFullSize;
    QString mAlt;
    AspectRatio::SharedPtr mAspectRatio; // optional
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ImagesViewImage>;
    static SharedPtr fromJson(const QJsonObject& json);
};
using ImagesViewImageList = std::vector<ImagesViewImage::SharedPtr>;

// app.bsky.embed.images#view
struct ImagesView
{
    ImagesViewImageList mImages; // max length 4

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ImagesView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.embed.images#view";
};

// app.bsky.embed.video#caption
struct VideoCaption
{
    QString mLang;
    Blob::SharedPtr mFile; // max 20,000 bytes mime: text/vtt

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<VideoCaption>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr int MAX_BYTES = 20'000;
    static constexpr char const* TYPE = "app.bsky.embed.video#caption";
};
using VideoCaptionList = std::vector<VideoCaption::SharedPtr>;

// app.bsky.embed.video
struct Video
{
    Blob::SharedPtr mVideo; // mime: video/mp4
    VideoCaptionList mCaptions;
    std::optional<QString> mAlt; // max 1000 graphemes, 10,000 bytes
    AspectRatio::SharedPtr mAspectRatio; // optional

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Video>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr int MAX_BYTES = 100'000'000;
    static constexpr char const* TYPE = "app.bsky.embed.video";
};

// app.bsky.embed.video#view
struct VideoView
{
    QString mCid;
    QString mPlaylist; // URI
    std::optional<QString> mThumbnail; // URI
    std::optional<QString> mAlt;
    AspectRatio::SharedPtr mAspectRatio; // optional
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<VideoView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.embed.video#view";
};

// app.bsky.embed.external#external
struct ExternalExternal
{
    QString mUri;
    QString mTitle;
    QString mDescription;
    Blob::SharedPtr mThumb; // optional: max 1,000,000 bytes mime: image/*

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ExternalExternal>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.embed.external
struct External
{
    ExternalExternal::SharedPtr mExternal;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<External>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.embed.external#viewExternal
struct ExternalViewExternal
{
    QString mUri;
    QString mTitle;
    QString mDescription;
    std::optional<QString> mThumb;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ExternalViewExternal>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.embed.external#view
struct ExternalView
{
    ExternalViewExternal::SharedPtr mExternal; // required

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ExternalView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.embed.external#view";
};

// app.bsky.embed.record
struct Record
{
    ComATProtoRepo::StrongRef::SharedPtr mRecord;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Record>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct RecordViewRecord;

// app.bsky.embed.record#viewNotFound
struct RecordViewNotFound
{
    QString mUri; // at-uri

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<RecordViewNotFound>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.embed.record#viewNotFound";
};

// app.bsky.embed.record#viewBlocked
struct RecordViewBlocked
{
    QString mUri; // at-uri

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<RecordViewBlocked>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.embed.record#viewBlocked";
};

// app.bsky.embed.record#viewDetached
struct RecordViewDetached
{
    QString mUri; // at-uri

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<RecordViewDetached>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.embed.record#viewDetached";
};

// app.bsky.embed.record#view
struct RecordView
{
    std::variant<std::shared_ptr<RecordViewRecord>,
                 RecordViewNotFound::SharedPtr,
                 RecordViewBlocked::SharedPtr,
                 RecordViewDetached::SharedPtr,
                 AppBskyFeed::GeneratorView::SharedPtr,
                 AppBskyGraph::ListView::SharedPtr,
                 AppBskyGraph::StarterPackViewBasic::SharedPtr,
                 AppBskyLabeler::LabelerView::SharedPtr> mRecord;
    RecordType mRecordType;

    QString mUnsupportedType; // not part of the spec

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<RecordView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.embed.record#view";
};

enum class EmbedType
{
    IMAGES,
    VIDEO,
    EXTERNAL,
    RECORD,
    RECORD_WITH_MEDIA,
    UNKNOWN
};

EmbedType stringToEmbedType(const QString& str);

// app.bsky.embed.recordWithMedia
struct RecordWithMedia
{
    Record::SharedPtr mRecord;
    std::variant<Images::SharedPtr, Video::SharedPtr, External::SharedPtr> mMedia;
    EmbedType mMediaType;
    QString mRawMediaType;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<RecordWithMedia>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.embed.recordWithMedia";
};

using EmbedUnion = std::variant<Images::SharedPtr,
                                Video::SharedPtr,
                                External::SharedPtr,
                                Record::SharedPtr,
                                RecordWithMedia::SharedPtr>;

struct Embed
{
    EmbedUnion mEmbed;
    EmbedType mType;
    QString mRawType;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Embed>;
    static SharedPtr fromJson(const QJsonObject& json);
};

enum class EmbedViewType
{
    IMAGES_VIEW,
    VIDEO_VIEW,
    EXTERNAL_VIEW,
    RECORD_VIEW,
    RECORD_WITH_MEDIA_VIEW,
    UNKNOWN
};

EmbedViewType stringToEmbedViewType(const QString& str);

// app.bsky.embed.recordWithMedia#view
struct RecordWithMediaView
{
    RecordView::SharedPtr mRecord;
    std::variant<ImagesView::SharedPtr, VideoView::SharedPtr, ExternalView::SharedPtr> mMedia;
    EmbedViewType mMediaType;
    QString mRawMediaType;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<RecordWithMediaView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.embed.recordWithMedia#view";
};

using EmbedViewUnion = std::variant<ImagesView::SharedPtr,
                                    VideoView::SharedPtr,
                                    ExternalView::SharedPtr,
                                    RecordView::SharedPtr,
                                    RecordWithMediaView::SharedPtr>;

struct EmbedView
{
    EmbedViewUnion mEmbed;
    EmbedViewType mType;
    QString mRawType;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<EmbedView>;
    static SharedPtr fromJson(const QJsonObject& json);
};
using EmbedViewList = std::vector<EmbedView::SharedPtr>;

}

namespace ATProto::AppBskyFeed {

// app.bsky.feed.post#replyRef
struct PostReplyRef
{
    ComATProtoRepo::StrongRef::SharedPtr mRoot; // required
    ComATProtoRepo::StrongRef::SharedPtr mParent; // required

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<PostReplyRef>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// Record types
namespace Record {

// app.bsky.feed.post
struct Post
{
    QString mText; // max 300 graphemes, 3000 bytes
    AppBskyRichtext::FacetList mFacets;
    PostReplyRef::SharedPtr mReply; // optional
    ATProto::AppBskyEmbed::Embed::SharedPtr mEmbed; // optional
    ComATProtoLabel::SelfLabels::SharedPtr mLabels; // optional
    std::vector<QString> mLanguages;
    QDateTime mCreatedAt;

    // Non-standard field added by bridgy-fed: https://github.com/snarfed/bridgy-fed/issues/1092
    std::optional<QString> mBridgyOriginalText;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Post>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}}

namespace ATProto::AppBskyEmbed {

// app.bsky.embed.record#viewRecord
struct RecordViewRecord
{
    QString mUri;
    QString mCid;
    AppBskyActor::ProfileViewBasic::SharedPtr mAuthor; // required
    std::variant<AppBskyFeed::Record::Post::SharedPtr,
                 AppBskyFeed::GeneratorView::SharedPtr,
                 AppBskyGraph::ListView::SharedPtr,
                 AppBskyLabeler::LabelerView::SharedPtr> mValue;
    RecordType mValueType;
    QString mRawValueType;
    ComATProtoLabel::LabelList mLabels;
    EmbedViewList mEmbeds;
    QDateTime mIndexedAt;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<RecordViewRecord>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.embed.record#viewRecord";
};

}
