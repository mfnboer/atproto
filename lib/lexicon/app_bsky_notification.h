// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include "app_bsky_feed.h"
#include "app_bsky_graph.h"
#include "com_atproto_label.h"
#include <QJsonObject>

namespace ATProto::AppBskyNotification {

// app.bsky.notification.declaration
struct Declaration
{
    AppBskyActor::AllowSubscriptionsType mAllowSubscriptions;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Declaration>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.notification.declaration";
};

// app.bsky.notification.defs#recordDeleted
struct RecordDeleted
{
    using SharedPtr = std::shared_ptr<RecordDeleted>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.notification.defs#recordDeleted";
};

// app.bsky.notification.defs#chatPreference
struct ChatPreference
{
    enum class IncludeType
    {
        ALL,
        ACCEPTED,
        UNKNOWN
    };

    static IncludeType stringToIncludeType(const QString& str);
    static QString includeTypeToString(IncludeType include, const QString& unknown);

    QString mRawInclude;
    IncludeType mInclude;
    bool mPush;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ChatPreference>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.notification.defs#chatPreference";
};

// app.bsky.notification.defs#filterablePreference
struct FilterablePreference
{
    enum class IncludeType
    {
        ALL,
        FOLLOWS,
        UNKNOWN
    };

    static IncludeType stringToIncludeType(const QString& str);
    static QString includeTypeToString(IncludeType include, const QString& unknown);

    QString mRawInclude;
    IncludeType mInclude;
    bool mList;
    bool mPush;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<FilterablePreference>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.notification.defs#filterablePreference";
};

// app.bsky.notification.defs#preference
struct Preference
{
    bool mList;
    bool mPush;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Preference>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.notification.defs#preference";
};

// app.bsky.notification.defs#preferences
struct Preferences
{
    ChatPreference::SharedPtr mChat;
    FilterablePreference::SharedPtr mFollow;
    FilterablePreference::SharedPtr mLike;
    FilterablePreference::SharedPtr mLikeViaRepost;
    FilterablePreference::SharedPtr mMention;
    FilterablePreference::SharedPtr mQuote;
    FilterablePreference::SharedPtr mReply;
    FilterablePreference::SharedPtr mRepost;
    FilterablePreference::SharedPtr mRepostViaRepost;
    Preference::SharedPtr mStarterpackJoined;
    Preference::SharedPtr mSubscribedPost;
    Preference::SharedPtr mUnverified;
    Preference::SharedPtr mVerified;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Preferences>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.notification.defs#preferences";
};

// app.bsky.notification.getPreferences#output
struct GetPreferencesOutput
{
    Preferences::SharedPtr mPreferences;

    using SharedPtr = std::shared_ptr<GetPreferencesOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.notification.defs#activitySubscription
struct ActivitySubscription
{
    bool mPost;
    bool mReply;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ActivitySubscription>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.notification.defs#activitySubscription";
};

// app.bsky.notification.defs#subjectActivitySubscription
struct SubjectActivitySubscription
{
    QString mSubject; // DID
    ActivitySubscription::SharedPtr mActivitySubscription;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<SubjectActivitySubscription>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.notification.defs#subjectActivitySubscription";
};

// app.bsky.notification.listActivitySubscriptions#output
struct ListActivitySubscriptionsOutput
{
    AppBskyActor::ProfileViewList mSubscriptions;
    std::optional<QString> mCursor;

    using SharedPtr = std::shared_ptr<ListActivitySubscriptionsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
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
    LIKE_VIA_REPOST,
    REPOST_VIA_REPOST,
    SUBSCRIBED_POST,
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
