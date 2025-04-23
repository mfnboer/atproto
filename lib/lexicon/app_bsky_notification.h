// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include "app_bsky_feed.h"
#include "app_bsky_graph.h"
#include "com_atproto_label.h"
#include <QJsonObject>

namespace ATProto::AppBskyNotification {

// app.bsky.notification.defs#recordDeleted
struct RecordDeleted
{
    using SharedPtr = std::shared_ptr<RecordDeleted>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.notification.defs#recordDeleted";
};

enum class NotificationReason
{
    LIKE,
    REPOST,
    FOLLOW,
    MENTION,
    REPLY,
    QUOTE,
    STARTERPACK_JOINED,
    VERIFIED,
    UNVERIFIED,
    UNKNOWN
};

NotificationReason stringToNotificationReason(const QString& str);
QString notificationReasonToString(NotificationReason reason);

// app.bsky.notification.listNotifications#notification
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
                 AppBskyGraph::Follow::SharedPtr,
                 AppBskyGraph::StarterPack::SharedPtr,
                 AppBskyGraph::Verification::SharedPtr,
                 RecordDeleted::SharedPtr> mRecord;
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
    bool mPriority = false;
    std::optional<QDateTime> mSeenAt;

    using SharedPtr = std::shared_ptr<ListNotificationsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}
