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
    AppBskyActor::ProfileView::SharedPtr mAuthor;
    NotificationReason mReason;
    QString mRawReason;
    std::optional<QString> mReasonSubject; // at-uri
    std::variant<AppBskyFeed::Record::Post::SharedPtr,
                 AppBskyFeed::Like::SharedPtr,
                 AppBskyFeed::Repost::SharedPtr,
                 AppBskyGraph::Follow::SharedPtr> mRecord;
    QString mRawRecordType;
    bool mIsRead;
    QDateTime mIndexedAt;
    ComATProtoLabel::LabelList mLabels;

    using SharedPtr = std::shared_ptr<Notification>;
    static SharedPtr fromJson(const QJsonObject& json);
};

using NotificationList = std::vector<Notification::SharedPtr>;

// app.bsky.notification.listNotifications#Output
struct ListNotificationsOutput
{
    std::optional<QString> mCursor;
    NotificationList mNotifications;

    using SharedPtr = std::shared_ptr<ListNotificationsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}
