// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "app_bsky_bookmark.h"
#include "xjson.h"

namespace ATProto::AppBskyBookmark {

BookmarkView::SharedPtr BookmarkView::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<BookmarkView>();
    const XJsonObject xjson(json);
    view->mSubject = xjson.getRequiredObject<ComATProtoRepo::StrongRef>("subject");
    view->mCreatedAt = xjson.getOptionalDateTime("createdAt");
    view->mItem = xjson.getRequiredVariant<AppBskyFeed::PostView, AppBskyFeed::NotFoundPost, AppBskyFeed::BlockedPost>("item");
    return view;
}

GetBookmarksOutput::SharedPtr GetBookmarksOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetBookmarksOutput>();
    const XJsonObject xjson(json);
    output->mBookmarks = xjson.getRequiredVector<BookmarkView>("bookmarks");
    output->mCursor = xjson.getOptionalString("cursor");
    return output;
}

}
