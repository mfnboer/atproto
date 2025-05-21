// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_embed_include.h"
#include "app_bsky_feed_include.h"
#include "app_bsky_graph_include.h"
#include "com_atproto_label.h"
#include "com_atproto_repo.h"
#include "lexicon.h"
#include <QJsonDocument>
#include <unordered_set>

namespace ATProto::AppBskyActor {

struct ProfileViewBasic;

// app.bsky.actor.defs#verificationView
struct VerificationView
{
    QString mIssuer; // DID
    QString mUri; // at-uri of the verification record
    bool mIsValid;
    QDateTime mCreatedAt;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<VerificationView>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

enum class VerifiedStatus
{
    VALID,
    INVALID,
    NONE,
    UNKNOWN
};

VerifiedStatus stringToVerifiedStatus(const QString& str);
QString verifiedStatusToString(VerifiedStatus status, const QString& unknown);

// app.bsky.actor.defs#verificationState
struct VerificationState
{
    VerificationView::List mVerifications;
    QString mRawVerifiedStatus;
    VerifiedStatus mVerifiedStatus;
    QString mRawTrustedVerifierStatus;
    VerifiedStatus mTrustedVerifierStatus;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<VerificationState>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#knownFollowers
struct KnownFollowers
{
    static constexpr int MAX_COUNT = 5;

    int mCount = 0;
    std::vector<std::shared_ptr<ProfileViewBasic>> mFollowers;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<KnownFollowers>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#viewerState
struct ViewerState
{
    bool mMuted = false;
    bool mBlockedBy = false;
    std::optional<QString> mBlocking;
    std::optional<QString> mFollowing;
    std::optional<QString> mFollowedBy;
    AppBskyGraph::ListViewBasic::SharedPtr mMutedByList;
    AppBskyGraph::ListViewBasic::SharedPtr mBlockingByList;
    KnownFollowers::SharedPtr mKnownFollowers;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ViewerState>;
    static SharedPtr fromJson(const QJsonObject& json);
};

enum class ActorStatus
{
    LIVE,
    UNKNOWN
};
ActorStatus stringToActorStatus(const QString& str);
QString actorStatusToString(ActorStatus status, const QString& unknown);

// app.bsky.actor.defs#statusView
struct StatusView
{
    using EmbedType = std::variant<AppBskyEmbed::ExternalView::SharedPtr>;

    QString mRawStatus;
    ActorStatus mStatus;
    QJsonObject mRecord;
    std::optional<EmbedType> mEmbed;
    std::optional<QDateTime> mExpiresAt;
    std::optional<bool> mIsActive;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<StatusView>;
    static SharedPtr fromJson(const QJsonObject& json);
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

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ProfileAssociatedChat>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#profileAssociated
struct ProfileAssociated
{
    int mLists = 0;
    int mFeeds = 0;
    int mStarterPacks = 0;
    bool mLabeler = false;
    ProfileAssociatedChat::SharedPtr mChat; // optional

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ProfileAssociated>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#profileViewBasic
struct ProfileViewBasic
{
    QString mDid;
    QString mHandle;
    std::optional<QString> mDisplayName; // max 64 graphemes, 640 bytes
    std::optional<QString> mAvatar; // URL
    ProfileAssociated::SharedPtr mAssociated; // optional
    ViewerState::SharedPtr mViewer; // optional
    ComATProtoLabel::LabelList mLabels;
    std::optional<QDateTime> mCreatedAt;
    VerificationState::SharedPtr mVerification; // optional
    StatusView::SharedPtr mStatus; // optional

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ProfileViewBasic>;
    static SharedPtr fromJson(const QJsonObject& json);
};

using ProfileViewBasicList = std::vector<ProfileViewBasic::SharedPtr>;

// app.bsky.actor.defs#profileView
struct ProfileView
{
    QString mDid;
    QString mHandle;
    std::optional<QString> mDisplayName; // max 64 graphemes, 640 bytes
    std::optional<QString> mAvatar; // URL
    ProfileAssociated::SharedPtr mAssociated; // optional
    std::optional<QString> mDescription; // max 256 graphemes, 2560 bytes
    std::optional<QDateTime> mIndexedAt;
    std::optional<QDateTime> mCreatedAt;
    ViewerState::SharedPtr mViewer; // optional
    ComATProtoLabel::LabelList mLabels;
    VerificationState::SharedPtr mVerification; // optional
    StatusView::SharedPtr mStatus; // optional

    QJsonObject toJson() const; // partial serialization

    using SharedPtr = std::shared_ptr<ProfileView>;
    static SharedPtr fromJson(const QJsonObject& json);
};

using ProfileViewList = std::vector<ProfileView::SharedPtr>;

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
    ProfileAssociated::SharedPtr mAssociated; // optional
    std::optional<QDateTime> mIndexedAt;
    std::optional<QDateTime> mCreatedAt;
    ViewerState::SharedPtr mViewer; // optional
    ComATProtoLabel::LabelList mLabels;
    ComATProtoRepo::StrongRef::SharedPtr mPinnedPost; // optional
    VerificationState::SharedPtr mVerification; // optional
    StatusView::SharedPtr mStatus; // optional

    using SharedPtr = std::shared_ptr<ProfileViewDetailed>;
    static SharedPtr fromJson(const QJsonObject& json);
};

using ProfileViewDetailedList = std::vector<ProfileViewDetailed::SharedPtr>;
void getProfileViewDetailedList(ProfileViewDetailedList& list, const QJsonObject& json);

// app.bsky.actor.profile
struct Profile
{
    std::optional<QString> mDisplayName;
    std::optional<QString> mDescription;
    Blob::SharedPtr mAvatar; // optional
    Blob::SharedPtr mBanner; // optional
    ComATProtoLabel::SelfLabels::SharedPtr mLabels; // optional
    ComATProtoRepo::StrongRef::SharedPtr mPinndedPost; // optional
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Profile>;
    static SharedPtr fromJson(const QJsonObject& json);
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

    using SharedPtr = std::shared_ptr<AdultContentPref>;
    static SharedPtr fromJson(const QJsonObject& json);
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

    using SharedPtr = std::shared_ptr<ContentLabelPref>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#savedFeedsPref
struct SavedFeedsPref
{
    std::vector<QString> mPinned;
    std::vector<QString> mSaved;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<SavedFeedsPref>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#personalDetailsPref
struct PersonalDetailsPref
{
    std::optional<QDateTime> mBirthDate;
    QJsonObject mJson;

    QJsonObject toJson() const { return mJson; } // TODO: encoding

    using SharedPtr = std::shared_ptr<PersonalDetailsPref>;
    static SharedPtr fromJson(const QJsonObject& json);
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

    using SharedPtr = std::shared_ptr<FeedViewPref>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#threadViewPref
struct ThreadViewPref
{
    std::optional<QString> mSort; // enum not implemented
    bool mPrioritizeFollowedUsers = false;
    QJsonObject mJson;

    QJsonObject toJson() const { return mJson; } // TODO: encoding

    using SharedPtr = std::shared_ptr<ThreadViewPref>;
    static SharedPtr fromJson(const QJsonObject& json);
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

enum class ActorTarget
{
    ALL,
    EXCLUDE_FOLLOWING,
    UNKNOWN
};
ActorTarget stringToActorTarget(const QString& str);
QString actorTargetToString(ActorTarget target);

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
    ActorTarget mActorTarget = ActorTarget::ALL;
    QString mRawActorTarget = QStringLiteral("all");
    std::optional<QDateTime> mExpiresAt;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<MutedWord>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#mutedWordsPref
struct MutedWordsPref
{
    // No unique ptrs as preferences will be copied in UserPreferences()
    std::vector<MutedWord> mItems;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<MutedWordsPref>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#labelerPrefItem
struct LabelerPrefItem
{
    QString mDid;
    QJsonObject mJson;

    QJsonObject toJson() const;
    bool operator==(const LabelerPrefItem& other) const { return mDid == other.mDid; }

    using SharedPtr = std::shared_ptr<LabelerPrefItem>;
    static SharedPtr fromJson(const QJsonObject& json);

    struct Hash
    {
        size_t operator()(const LabelerPrefItem& item) const
        {
            return ::std::hash<QString>()(item.mDid);
        }
    };
};

// app.bsky.actor.defs#labelersPref
struct LabelersPref
{
    // No unique ptrs as preferences will be copied in UserPreferences()
    std::unordered_set<LabelerPrefItem, LabelerPrefItem::Hash> mLabelers;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<LabelersPref>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#postInteractionSettingsPref
struct PostInteractionSettingsPref
{
    AppBskyFeed::ThreadgateRules mRules;
    bool mDisableEmbedding = false;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<PostInteractionSettingsPref>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.actor.defs#verificationPrefs
struct VerificationPrefs {
    bool mHideBadges = false;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<VerificationPrefs>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.actor.defs#verificationPrefs";
};

// Future preferences will be unknown. We store the json object, such that
// we can send it back unmodified for preference updates.
struct UnknownPref
{
    QJsonObject mJson;

    QJsonObject toJson() const { return mJson; }

    using SharedPtr = std::shared_ptr<UnknownPref>;
    static SharedPtr fromJson(const QJsonObject& json);
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
    POST_INTERACTION_SETTINGS,
    VERIFICATION,
    UNKNOWN
};
PreferenceType stringToPreferenceType(const QString& str);

using PreferenceItem = std::variant<AdultContentPref::SharedPtr,
                                    ContentLabelPref::SharedPtr,
                                    SavedFeedsPref::SharedPtr,
                                    PersonalDetailsPref::SharedPtr,
                                    FeedViewPref::SharedPtr,
                                    ThreadViewPref::SharedPtr,
                                    MutedWordsPref::SharedPtr,
                                    LabelersPref::SharedPtr,
                                    PostInteractionSettingsPref::SharedPtr,
                                    VerificationPrefs::SharedPtr,
                                    UnknownPref::SharedPtr>;

struct Preference
{
    PreferenceItem mItem;
    PreferenceType mType;
    QString mRawType;  

    using SharedPtr = std::shared_ptr<Preference>;
    static SharedPtr fromJson(const QJsonObject& json);
};

using PreferenceList = std::vector<Preference::SharedPtr>;

// app.bsky.actor.getPreferences#output
struct GetPreferencesOutput
{
    PreferenceList mPreferences;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<GetPreferencesOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.actor.searchActors#output
struct SearchActorsOutput
{
    std::optional<QString> mCursor;
    ProfileViewList mActors;

    using SharedPtr = std::shared_ptr<SearchActorsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.actor.searchActorsTypeahead#output
struct SearchActorsTypeaheadOutput
{
    ProfileViewBasicList mActors;

    using SharedPtr = std::shared_ptr<SearchActorsTypeaheadOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.actor.getSuggestion#output
struct GetSuggestionsOutput
{
    std::optional<QString> mCursor;
    ProfileViewList mActors;

    using SharedPtr = std::shared_ptr<GetSuggestionsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct GetSuggestedFollowsByActor
{
    ProfileViewList mSuggestions;

    using SharedPtr = std::shared_ptr<GetSuggestedFollowsByActor>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}
