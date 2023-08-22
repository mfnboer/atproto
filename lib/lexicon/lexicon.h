// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_richtext.h"
#include <QJsonDocument>

namespace ATProto {

// HTTP API (XRPC): error responses must contain json body with error and message fields.
struct ATProtoError
{
    QString mError;
    QString mMessage;

    using Ptr = std::unique_ptr<ATProtoError>;
    static Ptr fromJson(const QJsonDocument& json);
};

enum class RecordType
{
    APP_BSKY_EMBED_RECORD_VIEW_NOT_FOUND,
    APP_BSKY_EMBED_RECORD_VIEW_BLOCKED,
    APP_BSKY_EMBED_RECORD_VIEW_RECORD,

    APP_BSKY_FEED_POST,

    UNKNOWN
};

RecordType stringToRecordType(const QString& str);


namespace AppBskyFeed {

// Record types
namespace Record {

// app.bsky.feed.post
struct Post
{
    QString mText; // max 300 graphemes, 3000 bytes
    std::vector<AppBskyRichtext::Facet::Ptr> mFacets;
    // NOT IMPLEMENTED reply (it is detailed at the FeedViewPost level)
    // NOT IMPLEMENTED embed (it is detailed at the PostView level)
    QDateTime mCreatedAt;

    using Ptr = std::unique_ptr<Post>;
    static Ptr fromJson(const QJsonObject& json);
};

}}

}
