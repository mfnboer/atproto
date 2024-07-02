// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include "app_bsky_feed.h"
#include "app_bsky_graph.h"
#include "com_atproto_label.h"
#include <QJsonObject>

namespace ATProto::AppBskyNotification {

enum class NotificationReason
{
    LIKE,
    REPOST,
    FOLLOW,
    MENTION,
    REPLY,
    QUOTE,
    UNKNOWN
};

NotificationReason stringToNotificationReason(const QString& str);

// app.bsky.notification#notification
struct Notification
{
    QString mUri;
    QString mCid;
    AppBskyActor::ProfileView::Ptr mAuthor;
    NotificationReason mReason;
    QString mRawReason;
    std::optional<QString> mReasonSubject; // at-uri
    std::variant<AppBskyFeed::Record::Post::Ptr,
                 AppBskyFeed::Like::Ptr,
                 AppBskyFeed::Repost::Ptr,
                 AppBskyGraph::Follow::Ptr> mRecord;
    QString mRawRecordType;
    bool mIsRead;
    QDateTime mIndexedAt;
    std::vector<ComATProtoLabel::Label::Ptr> mLabels;

    using Ptr = std::unique_ptr<Notification>;
    static Ptr fromJson(const QJsonObject& json);
};

using NotificationList = std::vector<Notification::Ptr>;

// app.bsky.notification.listNotifications#Output
struct ListNotificationsOutput
{
    std::optional<QString> mCursor;
    NotificationList mNotifications;

    using Ptr = std::unique_ptr<ListNotificationsOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

}
