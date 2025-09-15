// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_notification.h"
#include "../xjson.h"
#include <unordered_map>

namespace ATProto::AppBskyNotification {

QJsonObject Declaration::toJson() const
{
    QJsonObject json{mJson};
    json.insert("$type", TYPE);
    json.insert("allowSubscriptions", AppBskyActor::allowSubscriptionsTypeToString(mAllowSubscriptions));
    return json;
}

Declaration::SharedPtr Declaration::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto declaration = std::make_shared<Declaration>();
    declaration->mAllowSubscriptions = AppBskyActor::stringToAllowSubscriptionsType(xjson.getRequiredString("allowSubscriptions"));
    declaration->mJson = json;
    return declaration;
}

ChatPreference::IncludeType ChatPreference::stringToIncludeType(const QString& str)
{
    static const std::unordered_map<QString, IncludeType> mapping = {
        { "all", IncludeType::ALL },
        { "accepted", IncludeType::ACCEPTED }
    };

    return stringToEnum(str, mapping, IncludeType::UNKNOWN);
}

QString ChatPreference::includeTypeToString(IncludeType include, const QString& unknown)
{
    static const std::unordered_map<IncludeType, QString> mapping = {
        { IncludeType::ALL, "all" },
        { IncludeType::ACCEPTED, "accepted" }
    };

    return enumToString(include, mapping, unknown);
}

QJsonObject ChatPreference::toJson() const
{
    QJsonObject json{mJson};
    json.insert("$type", TYPE);
    json.insert("include", includeTypeToString(mInclude, mRawInclude));
    json.insert("push", mPush);
    return json;
}

ChatPreference::SharedPtr ChatPreference::fromJson(const QJsonObject& json)
{
    auto pref = std::make_shared<ChatPreference>();
    XJsonObject xjson(json);
    pref->mRawInclude = xjson.getRequiredString("include");
    pref->mInclude = stringToIncludeType(pref->mRawInclude);
    pref->mPush = xjson.getRequiredBool("push");
    pref->mJson = json;
    return pref;
}

FilterablePreference::IncludeType FilterablePreference::stringToIncludeType(const QString& str)
{
    static const std::unordered_map<QString, IncludeType> mapping = {
        { "all", IncludeType::ALL },
        { "follows", IncludeType::FOLLOWS }
    };

    return stringToEnum(str, mapping, IncludeType::UNKNOWN);
}

QString FilterablePreference::includeTypeToString(IncludeType include, const QString& unknown)
{
    static const std::unordered_map<IncludeType, QString> mapping = {
        { IncludeType::ALL, "all" },
        { IncludeType::FOLLOWS, "follows" }
    };

    return enumToString(include, mapping, unknown);
}

QJsonObject FilterablePreference::toJson() const
{
    QJsonObject json{mJson};
    json.insert("$type", TYPE);
    json.insert("include", includeTypeToString(mInclude, mRawInclude));
    json.insert("list", mList);
    json.insert("push", mPush);
    return json;
}

FilterablePreference::SharedPtr FilterablePreference::fromJson(const QJsonObject& json)
{
    auto pref = std::make_shared<FilterablePreference>();
    XJsonObject xjson(json);
    pref->mRawInclude = xjson.getRequiredString("include");
    pref->mInclude = stringToIncludeType(pref->mRawInclude);
    pref->mList = xjson.getRequiredBool("list");
    pref->mPush = xjson.getRequiredBool("push");
    pref->mJson = json;
    return pref;
}

QJsonObject Preference::toJson() const
{
    QJsonObject json{mJson};
    json.insert("$type", TYPE);
    json.insert("list", mList);
    json.insert("push", mPush);
    return json;
}

Preference::SharedPtr Preference::fromJson(const QJsonObject& json)
{
    auto pref = std::make_shared<Preference>();
    XJsonObject xjson(json);
    pref->mList = xjson.getRequiredBool("list");
    pref->mPush = xjson.getRequiredBool("push");
    pref->mJson = json;
    return pref;
}

QJsonObject Preferences::toJson() const
{
    QJsonObject json{mJson};
    json.insert("$type", TYPE);
    json.insert("chat", mChat->toJson());
    json.insert("follow", mFollow->toJson());
    json.insert("like", mLike->toJson());
    json.insert("likeViaRepost", mLikeViaRepost->toJson());
    json.insert("mention", mMention->toJson());
    json.insert("quote", mQuote->toJson());
    json.insert("reply", mReply->toJson());
    json.insert("repost", mRepost->toJson());
    json.insert("repostViaRepost", mRepostViaRepost->toJson());
    json.insert("starterpackJoined", mStarterpackJoined->toJson());
    json.insert("subscribedPost", mSubscribedPost->toJson());
    json.insert("unverified", mUnverified->toJson());
    json.insert("verified", mVerified->toJson());
    return json;
}

Preferences::SharedPtr Preferences::fromJson(const QJsonObject& json)
{
    auto prefs = std::make_shared<Preferences>();
    XJsonObject xjson(json);
    prefs->mChat = xjson.getRequiredObject<ChatPreference>("chat");
    prefs->mFollow = xjson.getRequiredObject<FilterablePreference>("follow");
    prefs->mLike = xjson.getRequiredObject<FilterablePreference>("like");
    prefs->mLikeViaRepost = xjson.getRequiredObject<FilterablePreference>("likeViaRepost");
    prefs->mMention = xjson.getRequiredObject<FilterablePreference>("mention");
    prefs->mQuote = xjson.getRequiredObject<FilterablePreference>("quote");
    prefs->mReply = xjson.getRequiredObject<FilterablePreference>("reply");
    prefs->mRepost = xjson.getRequiredObject<FilterablePreference>("repost");
    prefs->mRepostViaRepost = xjson.getRequiredObject<FilterablePreference>("repostViaRepost");
    prefs->mStarterpackJoined = xjson.getRequiredObject<Preference>("starterpackJoined");
    prefs->mSubscribedPost = xjson.getRequiredObject<Preference>("subscribedPost");
    prefs->mUnverified = xjson.getRequiredObject<Preference>("unverified");
    prefs->mVerified = xjson.getRequiredObject<Preference>("verified");
    prefs->mJson = json;
    return prefs;
}

GetPreferencesOutput::SharedPtr GetPreferencesOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetPreferencesOutput>();
    XJsonObject xjson(json);
    output->mPreferences = xjson.getRequiredObject<Preferences>("preferences");
    return output;
}

QJsonObject ActivitySubscription::toJson() const
{
    QJsonObject json{mJson};
    json.insert("$type", TYPE);
    json.insert("post", mPost);
    json.insert("reply", mReply);
    return json;
}

ActivitySubscription::SharedPtr ActivitySubscription::fromJson(const QJsonObject& json)
{
    auto subscription = std::make_shared<ActivitySubscription>();
    XJsonObject xjson(json);
    subscription->mPost = xjson.getRequiredBool("post");
    subscription->mReply = xjson.getRequiredBool("reply");
    subscription->mJson = json;
    return subscription;
}

QJsonObject SubjectActivitySubscription::toJson() const
{
    QJsonObject json{mJson};
    json.insert("$type", TYPE);
    json.insert("subject", mSubject);
    XJsonObject::insertOptionalJsonObject<ActivitySubscription>(json, "activitySubscription", mActivitySubscription);
    return json;
}

SubjectActivitySubscription::SharedPtr SubjectActivitySubscription::fromJson(const QJsonObject& json)
{
    auto subject = std::make_shared<SubjectActivitySubscription>();
    XJsonObject xjson(json);
    subject->mSubject = xjson.getRequiredString("subject");
    subject->mActivitySubscription = xjson.getOptionalObject<ActivitySubscription>("activitySubscription");
    subject->mJson = json;
    return subject;
}

ListActivitySubscriptionsOutput::SharedPtr ListActivitySubscriptionsOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<ListActivitySubscriptionsOutput>();
    output->mSubscriptions = xjson.getRequiredVector<AppBskyActor::ProfileView>("subscriptions");
    output->mCursor = xjson.getOptionalString("cursor");
    return output;
}

NotificationReason stringToNotificationReason(const QString& str)
{
    static const std::unordered_map<QString, NotificationReason> mapping = {
        { "like", NotificationReason::LIKE },
        { "repost", NotificationReason::REPOST },
        { "follow", NotificationReason::FOLLOW },
        { "mention", NotificationReason::MENTION },
        { "reply", NotificationReason::REPLY },
        { "quote", NotificationReason::QUOTE },
        { "starterpack-joined", NotificationReason::STARTERPACK_JOINED },
        { "verified", NotificationReason::VERIFIED },
        { "unverified", NotificationReason::UNVERIFIED },
        { "like-via-repost", NotificationReason::LIKE_VIA_REPOST },
        { "repost-via-repost", NotificationReason::REPOST_VIA_REPOST },
        { "subscribed-post", NotificationReason::SUBSCRIBED_POST }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    return NotificationReason::UNKNOWN;
}

QString notificationReasonToString(NotificationReason reason)
{
    static const std::unordered_map<NotificationReason, QString> mapping = {
        { NotificationReason::LIKE, "like" },
        { NotificationReason::REPOST, "repost" },
        { NotificationReason::FOLLOW, "follow" },
        { NotificationReason::MENTION, "mention" },
        { NotificationReason::REPLY, "reply" },
        { NotificationReason::QUOTE, "quote" },
        { NotificationReason::STARTERPACK_JOINED, "starterpack-joined" },
        { NotificationReason::VERIFIED, "verified" },
        { NotificationReason::UNVERIFIED, "unverified" },
        { NotificationReason::LIKE_VIA_REPOST, "like-via-repost" },
        { NotificationReason::REPOST_VIA_REPOST, "repost-via-repost" },
        { NotificationReason::SUBSCRIBED_POST, "subscribed-post" }
    };

    const auto it = mapping.find(reason);
    Q_ASSERT(it != mapping.end());
    if (it != mapping.end())
        return it->second;

    return {};
}

RecordDeleted::SharedPtr RecordDeleted::fromJson(const QJsonObject&)
{
    return std::make_shared<RecordDeleted>();
}

Notification::SharedPtr Notification::fromJson(const QJsonObject& json)
{
    auto notification = std::make_shared<Notification>();
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
        case NotificationReason::LIKE_VIA_REPOST:
            notification->mRecord = AppBskyFeed::Like::fromJson(recordJson);
            break;
        case NotificationReason::REPOST:
        case NotificationReason::REPOST_VIA_REPOST:
            notification->mRecord = AppBskyFeed::Repost::fromJson(recordJson);
            break;
        case NotificationReason::FOLLOW:
            notification->mRecord = AppBskyGraph::Follow::fromJson(recordJson);
            break;
        case NotificationReason::MENTION:
        case NotificationReason::REPLY:
        case NotificationReason::QUOTE:
        case NotificationReason::SUBSCRIBED_POST:
            notification->mRecord = AppBskyFeed::Record::Post::fromJson(recordJson);
            break;
        case ATProto::AppBskyNotification::NotificationReason::STARTERPACK_JOINED:
            notification->mRecord = AppBskyGraph::StarterPack::fromJson(recordJson);
            break;
        case NotificationReason::VERIFIED:
            if (notification->mRawRecordType == AppBskyGraph::Verification::TYPE)
                notification->mRecord = AppBskyGraph::Verification::fromJson(recordJson);
            else
                notification->mRecord = RecordDeleted::fromJson(recordJson);

            break;
        case NotificationReason::UNVERIFIED:
            notification->mRecord = RecordDeleted::fromJson(recordJson);
            break;
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

static void getNotificationList(Notification::List& list, const QJsonObject& json)
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

ListNotificationsOutput::SharedPtr ListNotificationsOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<ListNotificationsOutput>();
    XJsonObject xjson(json);
    output->mCursor = xjson.getOptionalString("cursor");
    getNotificationList(output->mNotifications, json);
    output->mPriority = xjson.getOptionalBool("priority", false);
    output->mSeenAt = xjson.getOptionalDateTime("seenAt");
    return output;
}

}
