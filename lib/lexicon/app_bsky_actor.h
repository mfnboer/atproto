// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_graph_include.h"
#include "com_atproto_label.h"
#include "lexicon.h"
#include <QJsonDocument>
#include <unordered_set>

namespace ATProto::AppBskyActor {

// app.bsky.actor.defs#viewerState
struct ViewerState
{
    bool mMuted = false;
    bool mBlockedBy = false;
    std::optional<QString> mBlocking;
    std::optional<QString> mFollowing;
    std::optional<QString> mFollowedBy;
    std::unique_ptr<AppBskyGraph::ListViewBasic> mMutedByList;
    std::unique_ptr<AppBskyGraph::ListViewBasic> mBlockingByList;

    using Ptr = std::unique_ptr<ViewerState>;
    static Ptr fromJson(const QJsonObject& json);
};

// chat.bsky.actor.declaration enum
enum class AllowIncomingType
{
    ALL,
    NONE,
    FOLLOWING
};
AllowIncomingType stringToAllowIncomingType(const QString& str);
QString allowIncomingTypeToString(AllowIncomingType allowIncoming);

struct ProfileAssociatedChat
{
    AppBskyActor::AllowIncomingType mAllowIncoming = AllowIncomingType::FOLLOWING;

    using Ptr = std::unique_ptr<ProfileAssociatedChat>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#profileAssociated
struct ProfileAssociated
{
    int mLists = 0;
    int mFeeds = 0;
    bool mLabeler = false;
    ProfileAssociatedChat::Ptr mChat; // optional

    using Ptr = std::unique_ptr<ProfileAssociated>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#profileViewBasic
struct ProfileViewBasic
{
    QString mDid;
    QString mHandle;
    std::optional<QString> mDisplayName; // max 64 graphemes, 640 bytes
    std::optional<QString> mAvatar; // URL
    ProfileAssociated::Ptr mAssociated; // optional
    ViewerState::Ptr mViewer; // optional
    std::vector<ComATProtoLabel::Label::Ptr> mLabels;

    QJsonObject toJson() const; // partial serialization

    using Ptr = std::unique_ptr<ProfileViewBasic>;
    static Ptr fromJson(const QJsonObject& json);
};

using ProfileViewBasicList = std::vector<ProfileViewBasic::Ptr>;

// app.bsky.actor.defs#profileView
struct ProfileView
{
    QString mDid;
    QString mHandle;
    std::optional<QString> mDisplayName; // max 64 graphemes, 640 bytes
    std::optional<QString> mAvatar; // URL
    ProfileAssociated::Ptr mAssociated; // optional
    std::optional<QString> mDescription; // max 256 graphemes, 2560 bytes
    std::optional<QDateTime> mIndexedAt;
    ViewerState::Ptr mViewer; // optional
    std::vector<ComATProtoLabel::Label::Ptr> mLabels;

    QJsonObject toJson() const; // partial serialization

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
    ProfileAssociated::Ptr mAssociated; // optional
    std::optional<QDateTime> mIndexedAt;
    ViewerState::Ptr mViewer; // optional
    std::vector<ComATProtoLabel::Label::Ptr> mLabels;

    using SharedPtr = std::shared_ptr<ProfileViewDetailed>;
    using Ptr = std::unique_ptr<ProfileViewDetailed>;
    static Ptr fromJson(const QJsonObject& json);
};

using ProfileViewDetailedList = std::vector<ProfileViewDetailed::Ptr>;
void getProfileViewDetailedList(ProfileViewDetailedList& list, const QJsonObject& json);

// app.bsky.actor.profile
struct Profile
{
    std::optional<QString> mDisplayName;
    std::optional<QString> mDescription;
    Blob::Ptr mAvatar; // optional
    Blob::Ptr mBanner; // optional
    ComATProtoLabel::SelfLabels::Ptr mLabels; // optional
    QJsonObject mJson;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Profile>;
    static Ptr fromJson(const QJsonObject& json);
};

// For the user preferences we store the received json object.
// When sending preferences we take this object and update the modified parts.
// This way we can send back any unknown parts (from future proto updates) unchanged.
// Unfortunately you cannot send partial prefereces. Anything not sent will be
// erased by Bluesky, i.e. if you do not send the birthdate, then the birthdate will
// be erased.

// app.bsky.actor.defs#adultContentPref
struct AdultContentPref
{
    bool mEnabled = false;
    QJsonObject mJson;

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
    static QString visibilityToString(Visibility visibility, const QString& unknown);

    std::optional<QString> mLabelerDid; // not set means global label
    QString mLabel;
    Visibility mVisibility;
    QString mRawVisibility;
    QJsonObject mJson;

    bool isGlobal() const { return !mLabelerDid; }
    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<ContentLabelPref>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#savedFeedsPref
struct SavedFeedsPref
{
    std::vector<QString> mPinned;
    std::vector<QString> mSaved;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<SavedFeedsPref>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#personalDetailsPref
struct PersonalDetailsPref
{
    std::optional<QDateTime> mBirthDate;
    QJsonObject mJson;

    QJsonObject toJson() const { return mJson; } // TODO: encoding

    using Ptr = std::unique_ptr<PersonalDetailsPref>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#feedViewPref
struct FeedViewPref
{
    QString mFeed;
    bool mHideReplies = false;
    bool mHideRepliesByUnfollowed = false;
    int mHideRepliesByLikeCount = 0;
    bool mHideReposts = false;
    bool mHideQuotePosts = false;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<FeedViewPref>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#threadViewPref
struct ThreadViewPref
{
    std::optional<QString> mSort; // enum not implemented
    bool mPrioritizeFollowedUsers = false;
    QJsonObject mJson;

    QJsonObject toJson() const { return mJson; } // TODO: encoding

    using Ptr = std::unique_ptr<ThreadViewPref>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#mutedWordTarget
enum class MutedWordTarget
{
    CONTENT,
    TAG,
    UNKNOWN
};
MutedWordTarget stringToMutedWordTarget(const QString& str);
QString mutedWordTargetToString(MutedWordTarget target);

// app.bsky.actor.defs#mutedWord
struct MutedWord
{
    struct Target
    {
        MutedWordTarget mTarget;
        QString mRawTarget;
    };

    QString mValue;
    std::vector<Target> mTargets;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<MutedWord>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#mutedWordsPref
struct MutedWordsPref
{
    // No unique ptrs as preferences will be copied in UserPreferences()
    std::vector<MutedWord> mItems;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<MutedWordsPref>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#labelerPrefItem
struct LabelerPrefItem
{
    QString mDid;
    QJsonObject mJson;

    QJsonObject toJson() const;
    bool operator==(const LabelerPrefItem& other) const { return mDid == other.mDid; }

    using Ptr = std::unique_ptr<LabelerPrefItem>;
    static Ptr fromJson(const QJsonObject& json);

    struct Hash
    {
        size_t operator()(const LabelerPrefItem& item) const
        {
            return ::std::hash<QString>()(item.mDid);
        }
    };
};

struct LabelersPref
{
    // No unique ptrs as preferences will be copied in UserPreferences()
    std::unordered_set<LabelerPrefItem, LabelerPrefItem::Hash> mLabelers;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<LabelersPref>;
    static Ptr fromJson(const QJsonObject& json);
};

// Future preferences will be unknown. We store the json object, such that
// we can send it back unmodified for preference updates.
struct UnknownPref
{
    QJsonObject mJson;

    QJsonObject toJson() const { return mJson; }

    using Ptr = std::unique_ptr<UnknownPref>;
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
    MUTED_WORDS,
    LABELERS,
    UNKNOWN
};
PreferenceType stringToPreferenceType(const QString& str);

using PreferenceItem = std::variant<AdultContentPref::Ptr,
                                    ContentLabelPref::Ptr,
                                    SavedFeedsPref::Ptr,
                                    PersonalDetailsPref::Ptr,
                                    FeedViewPref::Ptr,
                                    ThreadViewPref::Ptr,
                                    MutedWordsPref::Ptr,
                                    LabelersPref::Ptr,
                                    UnknownPref::Ptr>;

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

// app.bsky.actor.searchActors#output
struct SearchActorsOutput
{
    std::optional<QString> mCursor;
    ProfileViewList mActors;

    using Ptr = std::unique_ptr<SearchActorsOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.searchActorsTypeahead#output
struct SearchActorsTypeaheadOutput
{
    ProfileViewBasicList mActors;

    using Ptr = std::unique_ptr<SearchActorsTypeaheadOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.actor.getSuggestion#output
struct GetSuggestionsOutput
{
    std::optional<QString> mCursor;
    ProfileViewList mActors;

    using Ptr = std::unique_ptr<GetSuggestionsOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

struct GetSuggestedFollowsByActor
{
    ProfileViewList mSuggestions;

    using Ptr = std::unique_ptr<GetSuggestedFollowsByActor>;
    static Ptr fromJson(const QJsonObject& json);
};

}
