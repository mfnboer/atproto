// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_feed_include.h"
#include "com_atproto_label.h"
#include "com_atproto_repo.h"
#include <QJsonDocument>

namespace ATProto::AppBskyDraft {

// app.bsky.draft.defs#draftEmbedLocalRef
struct DraftEmbedLocalRef
{
    QString mPath; // Local, on-device ref to file to be embedded.

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<DraftEmbedLocalRef>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr int MIN_PATH_LEN = 1;
    static constexpr int MAX_PATH_LEN = 1024;
    static constexpr char const* TYPE = "app.bsky.draft.defs#draftEmbedLocalRef";
};

// app.bsky.draft.defs#draftEmbedCaption
struct DraftEmbedCaption
{
    QString mLang;
    QString mContent;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<DraftEmbedCaption>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.draft.defs#draftEmbedCaption";
};

// app.bsky.draft.defs#draftEmbedImage
struct DraftEmbedImage
{
    DraftEmbedLocalRef::SharedPtr mLocalRef;
    std::optional<QString> mAlt;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<DraftEmbedImage>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr int MAX_ALT_GRAPHEMES = 2000;
    static constexpr char const* TYPE = "app.bsky.draft.defs#draftEmbedImage";
};

// app.bsky.draft.defs#draftEmbedVideo
struct DraftEmbedVideo
{
    DraftEmbedLocalRef::SharedPtr mLocalRef;
    std::optional<QString> mAlt;
    DraftEmbedCaption::List mCaptions; // optional

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<DraftEmbedVideo>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr int MAX_ALT_GRAPHEMES = 2000;
    static constexpr int MAX_CAPTIONS = 20;
    static constexpr char const* TYPE = "app.bsky.draft.defs#draftEmbedVideo";
};

// app.bsky.draft.defs#draftEmbedExternal
struct DraftEmbedExternal
{
    QString mUri;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<DraftEmbedExternal>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.draft.defs#draftEmbedExternal";
};

// app.bsky.draft.defs#draftEmbedRecord
struct DraftEmbedRecord
{
    ComATProtoRepo::StrongRef::SharedPtr mRecord;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<DraftEmbedRecord>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.draft.defs#draftEmbedRecord";
};

// app.bsky.draft.defs#draftPost
struct DraftPost
{
    QString mText;
    ComATProtoLabel::SelfLabels::SharedPtr mLabels; // optional
    DraftEmbedImage::List mEmbedImages; // optional max=4
    DraftEmbedVideo::List mEmbedVideos; // optional max=1
    DraftEmbedExternal::List mEmbedExternals; // optional max=1
    DraftEmbedRecord::List mEmbedRecords; // optional max=1

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<DraftPost>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr int MAX_TEXT_GRAPHEMES = 300;
    static constexpr int MAX_TEXT_BYTES = 3000;
    static constexpr int MAX_EMBED_IMAGES = 1;
    static constexpr int MAX_EMBED_VIDEOS = 1;
    static constexpr int MAX_EMBED_EXTERNALS = 1;
    static constexpr int MAX_EMBED_RECORDS = 1;
    static constexpr char const* TYPE = "app.bsky.draft.defs#draftPost";
};

// app.bsky.draft.defs#draft
struct Draft
{
    DraftPost::List mPosts; // min=1 max=100
    std::vector<QString> mLangs; // optional max=3
    bool mDisableEmbedding = false;
    AppBskyFeed::ThreadgateRules mThreadgateRules;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Draft>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr int MIN_POSTS = 1;
    static constexpr int MAX_POSTS = 100;
    static constexpr int MAX_LANGS = 3;
    static constexpr char const* TYPE = "app.bsky.draft.defs#draft";
};

// app.bsky.draft.defs#draftWithId
struct DraftWithId
{
    QString mId;
    Draft::SharedPtr mDraft;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<DraftWithId>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.draft.defs#draftWithId";
};

// app.bsky.draft.defs#draftView
struct DraftView
{
    QString mId;
    Draft::SharedPtr mDraft;
    QDateTime mCreatedAt;
    QDateTime mUpdatedAt;

    using SharedPtr = std::shared_ptr<DraftView>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.draft.getDrafts#output
struct GetDraftsOutput
{
    DraftView::List mDrafts;

    using SharedPtr = std::shared_ptr<GetDraftsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.draft.createDraft#output
struct CreateDraftOutput
{
    QString mId;

    using SharedPtr = std::shared_ptr<CreateDraftOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}
