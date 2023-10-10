// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "com_atproto_label.h"
#include <QJsonDocument>

namespace ATProto::AppBskyActor {

// app.bsky.actor.defs#viewerState
struct ViewerState
{
    bool mMuted = false;
    bool mBlockedBy = false;
    std::optional<QString> mBlocking;
    std::optional<QString> mFollowing;
    std::optional<QString> mFollowedBy;
    // NOT IMPLEMENTED: mutedByList

    using Ptr = std::unique_ptr<ViewerState>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#profileViewBasic
struct ProfileViewBasic
{
    QString mDid;
    QString mHandle;
    std::optional<QString> mDisplayName; // max 64 graphemes, 640 bytes
    std::optional<QString> mAvatar; // URL
    ViewerState::Ptr mViewer; // optional
    std::vector<ComATProtoLabel::Label::Ptr> mLabels;

    using Ptr = std::unique_ptr<ProfileViewBasic>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#profileView
struct ProfileView
{
    QString mDid;
    QString mHandle;
    std::optional<QString> mDisplayName; // max 64 graphemes, 640 bytes
    std::optional<QString> mAvatar; // URL
    std::optional<QString> mDescription; // max 256 graphemes, 2560 bytes
    std::optional<QDateTime> mIndexedAt;
    ViewerState::Ptr mViewer; // optional
    std::vector<ComATProtoLabel::Label::Ptr> mLabels;

    using SharedPtr = std::shared_ptr<ProfileView>;
    using Ptr = std::unique_ptr<ProfileView>;
    static Ptr fromJson(const QJsonObject& json);
};

using ProfileViewList = std::vector<ProfileView::Ptr>;

// app.bsky.actor.defs#profileViewDetailed
struct ProfileViewDetailed
{
    QString mDid;
    QString mHandle;
    std::optional<QString> mDisplayName; // max 64 graphemes, 640 bytes
    std::optional<QString> mAvatar; // URL
    std::optional<QString> mBanner; // URL
    std::optional<QString> mDescription; // max 256 graphemes, 2560 bytes
    int mFollowersCount = 0;
    int mFollowsCount = 0;
    int mPostsCount = 0;
    std::optional<QDateTime> mIndexedAt;
    ViewerState::Ptr mViewer; // optional
    std::vector<ComATProtoLabel::Label::Ptr> mLabels;

    using SharedPtr = std::shared_ptr<ProfileViewDetailed>;
    using Ptr = std::unique_ptr<ProfileViewDetailed>;
    static Ptr fromJson(const QJsonDocument& json);
};

// app.bsky.actor.defs#adultContentPref
struct AdultContentPref
{
    bool mEnabled = false;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<AdultContentPref>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#contentLabelPref
struct ContentLabelPref
{
    enum class Visibility
    {
        SHOW,
        WARN,
        HIDE,
        UNKNOWN
    };
    static Visibility stringToVisibility(const QString& str);
    static QString visibilityToString(Visibility visibility);

    QString mLabel;
    Visibility mVisibility;
    QString mRawVisibility;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<ContentLabelPref>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#savedFeedsPref
struct SavedFeedsPref
{
    std::vector<QString> mPinned;
    std::vector<QString> mSaved;
    QJsonObject mJson; // Temporary

    QJsonObject toJson() const { return mJson; }

    using Ptr = std::unique_ptr<SavedFeedsPref>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#personalDetailsPref
struct PersonalDetailsPref
{
    std::optional<QDateTime> mBirthDate;
    QJsonObject mJson; // Temporary

    QJsonObject toJson() const { return mJson; }

    using Ptr = std::unique_ptr<PersonalDetailsPref>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#feedViewPref
struct FeedViewPref
{
    QString mFeed;
    bool mHideReplies = false;
    bool mHideRepliesByUnfollowed = true;
    int mHideRepliesByLikeCount = 0;
    bool mHideReposts = false;
    bool mHideQuotePosts = false;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<FeedViewPref>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#threadViewPref
struct ThreadViewPref
{
    std::optional<QString> mSort; // enum not implemented
    bool mPrioritizeFollowedUsers = false;
    QJsonObject mJson; // Temporary

    QJsonObject toJson() const { return mJson; }

    using Ptr = std::unique_ptr<ThreadViewPref>;
    static Ptr fromJson(const QJsonObject& json);
};

enum class PreferenceType
{
    ADULT_CONTENT,
    CONTENT_LABEL,
    SAVED_FEEDS,
    PERSONAL_DETAILS,
    FEED_VIEW,
    THREAD_VIEW,
    UNKNOWN
};
PreferenceType stringToPreferenceType(const QString& str);

using PreferenceItem = std::variant<AdultContentPref::Ptr,
                                    ContentLabelPref::Ptr,
                                    SavedFeedsPref::Ptr,
                                    PersonalDetailsPref::Ptr,
                                    FeedViewPref::Ptr,
                                    ThreadViewPref::Ptr>;

struct Preference
{
    PreferenceItem mItem;
    PreferenceType mType;
    QString mRawType;

    using Ptr = std::unique_ptr<Preference>;
    static Ptr fromJson(const QJsonObject& json);
};

using PreferenceList = std::vector<Preference::Ptr>;

// app.bsky.actor.getPreferences#output
struct GetPreferencesOutput
{
    PreferenceList mPreferences;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<GetPreferencesOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

}
