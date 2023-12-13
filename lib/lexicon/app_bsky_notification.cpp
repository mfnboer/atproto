// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_notification.h"
#include "../xjson.h"
#include <unordered_map>

namespace ATProto::AppBskyNotification {

NotificationReason stringToNotificationReason(const QString& str)
{
    static const std::unordered_map<QString, NotificationReason> mapping = {
        { "like", NotificationReason::LIKE },
        { "repost", NotificationReason::REPOST },
        { "follow", NotificationReason::FOLLOW },
        { "mention", NotificationReason::MENTION },
        { "reply", NotificationReason::REPLY },
        { "quote", NotificationReason::QUOTE }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    return NotificationReason::UNKNOWN;
}

Notification::Ptr Notification::fromJson(const QJsonObject& json)
{
    auto notification = std::make_unique<Notification>();
    XJsonObject xjson(json);
    notification->mUri = xjson.getRequiredString("uri");
    notification->mCid = xjson.getRequiredString("cid");
    notification->mAuthor = xjson.getRequiredObject<AppBskyActor::ProfileView>("author");
    notification->mRawReason = xjson.getRequiredString("reason");
    notification->mReason = stringToNotificationReason(notification->mRawReason);
    notification->mReasonSubject = xjson.getOptionalString("reasonSubject");
    auto recordJson = xjson.getRequiredJsonObject("record");
    notification->mRawRecordType = XJsonObject(recordJson).getOptionalString("$type").value_or("TypeMissing");

    try {
        switch (notification->mReason)
        {
        case NotificationReason::LIKE:
            notification->mRecord = AppBskyFeed::Like::fromJson(recordJson);
            break;
        case NotificationReason::REPOST:
            notification->mRecord = AppBskyFeed::Repost::fromJson(recordJson);
            break;
        case NotificationReason::FOLLOW:
            notification->mRecord = AppBskyGraph::Follow::fromJson(recordJson);
            break;
        case NotificationReason::MENTION:
        case NotificationReason::REPLY:
        case NotificationReason::QUOTE:
            notification->mRecord = AppBskyFeed::Record::Post::fromJson(recordJson);
            break;
        case NotificationReason::INVITE_CODE_USED:
        case NotificationReason::UNKNOWN:
            qWarning() << "Unknow notification reason:" << notification->mRawReason;
            break;
        }
    } catch (InvalidJsonException& e) {
        qWarning() << "Failed to parse record:" << recordJson;
    }

    notification->mIsRead = xjson.getRequiredBool("isRead");
    notification->mIndexedAt = xjson.getRequiredDateTime("indexedAt");
    ComATProtoLabel::getLabels(notification->mLabels, json);
    return notification;
}

static void getNotificationList(NotificationList& list, const QJsonObject& json)
{
    XJsonObject xjson(json);

    try {
        const QJsonArray& listArray = xjson.getRequiredArray("notifications");
        list.reserve(listArray.size());

        for (const auto& notificationJson : listArray)
        {
            if (!notificationJson.isObject())
            {
                qWarning() << "PROTO ERROR invalid list element: not an object";
                qInfo() << json;
                continue;
            }

            try {
                auto notification = Notification::fromJson(notificationJson.toObject());
                list.push_back(std::move(notification));
            } catch (InvalidJsonException& e) {
                qWarning() << "PROTO ERROR invalid list element:" << e.msg();
                qInfo() << json;
                continue;
            }
        }
    } catch (InvalidJsonException& e) {
        qWarning() << "PROTO ERROR invalid list:" << e.msg();
        qInfo() << json;
    }
}

ListNotificationsOutput::Ptr ListNotificationsOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_unique<ListNotificationsOutput>();
    XJsonObject xjson(json);
    output->mCursor = xjson.getOptionalString("cursor");
    getNotificationList(output->mNotifications, json);
    return output;
}

}
