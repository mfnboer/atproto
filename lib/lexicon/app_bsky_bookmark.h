// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_feed.h"
#include "com_atproto_repo.h"
#include <QJsonDocument>

namespace ATProto::AppBskyBookmark {

// app.bsky.bookmark.defs#bookmarkView
struct BookmarkView
{
    ComATProtoRepo::StrongRef::SharedPtr mSubject;
    std::optional<QDateTime> mCreatedAt;
    std::variant<AppBskyFeed::PostView::SharedPtr,
                 AppBskyFeed::NotFoundPost::SharedPtr,
                 AppBskyFeed::BlockedPost::SharedPtr> mItem;

    using SharedPtr = std::shared_ptr<BookmarkView>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.bookmark.getBookmarks#output
struct GetBookmarksOutput
{
    BookmarkView::List mBookmarks;
    std::optional<QString> mCursor;

    using SharedPtr = std::shared_ptr<GetBookmarksOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}
