// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_actor.h"
#include "../xjson.h"
#include <unordered_map>

namespace ATProto::AppBskyActor {

AllowIncomingType stringToAllowIncomingType(const QString& str)
{
    static const std::unordered_map<QString, AllowIncomingType> mapping = {
        { "all", AllowIncomingType::ALL },
        { "none", AllowIncomingType::NONE },
        { "following", AllowIncomingType::FOLLOWING }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    return AllowIncomingType::NONE;
}

QString allowIncomingTypeToString(AllowIncomingType allowIncoming)
{
    static const std::unordered_map<AllowIncomingType, QString> mapping = {
        { AllowIncomingType::ALL, "all" },
        { AllowIncomingType::NONE, "none" },
        { AllowIncomingType::FOLLOWING, "following" }
    };

    const auto it = mapping.find(allowIncoming);
    Q_ASSERT(it != mapping.end());

    if (it == mapping.end())
    {
        qWarning() << "Unknown allow incoming type:" << int(allowIncoming);
        return "none";
    }

    return it->second;
}

AllowSubscriptionsType stringToAllowSubscriptionsType(const QString& str)
{
    static const std::unordered_map<QString, AllowSubscriptionsType> mapping = {
        { "followers", AllowSubscriptionsType::FOLLOWERS },
        { "mutuals", AllowSubscriptionsType::MUTUALS },
        { "none", AllowSubscriptionsType::NONE }
    };

    return stringToEnum(str, mapping, AllowSubscriptionsType::NONE);
}

QString allowSubscriptionsTypeToString(AllowSubscriptionsType allowSubscriptions)
{
    static const std::unordered_map<AllowSubscriptionsType, QString> mapping = {
        { AllowSubscriptionsType::FOLLOWERS, "followers" },
        { AllowSubscriptionsType::MUTUALS, "mutuals" },
        { AllowSubscriptionsType::NONE, "none" }
    };

    return enumToString(allowSubscriptions, mapping, "none");
}

VerifiedStatus stringToVerifiedStatus(const QString& str)
{
    static const std::unordered_map<QString, VerifiedStatus> mapping = {
        { "valid", VerifiedStatus::VALID },
        { "invalid", VerifiedStatus::INVALID },
        { "none", VerifiedStatus::NONE }
    };

    return stringToEnum(str, mapping, VerifiedStatus::UNKNOWN);
}

QString verifiedStatusToString(VerifiedStatus status, const QString& unknown)
{
    static const std::unordered_map<VerifiedStatus, QString> mapping = {
        { VerifiedStatus::VALID, "valid" },
        { VerifiedStatus::INVALID, "invalid" },
        { VerifiedStatus::NONE, "none" }
    };

    return enumToString(status, mapping, unknown);
}

QJsonObject VerificationView::toJson() const
{
    QJsonObject json;
    json.insert("issuer", mIssuer);
    json.insert("uri", mUri);
    json.insert("isValid", mIsValid);
    json.insert("createdAt", mCreatedAt.toString(Qt::ISODateWithMs));
    return json;
}

VerificationView::SharedPtr VerificationView::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<VerificationView>();
    XJsonObject xjson(json);
    view->mIssuer = xjson.getRequiredString("issuer");
    view->mUri = xjson.getRequiredString("uri");
    view->mIsValid = xjson.getRequiredBool("isValid");
    view->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return view;
}

QJsonObject VerificationState::toJson() const
{
    QJsonObject json;
    json.insert("verifications", XJsonObject::toJsonArray<VerificationView>(mVerifications));
    json.insert("verifiedStatus", verifiedStatusToString(mVerifiedStatus, mRawVerifiedStatus));
    json.insert("trustedVerifierStatus", verifiedStatusToString(mTrustedVerifierStatus, mRawTrustedVerifierStatus));
    return json;
}

VerificationState::SharedPtr VerificationState::fromJson(const QJsonObject& json)
{
    auto verificationState = std::make_shared<VerificationState>();
    XJsonObject xjson(json);
    verificationState->mVerifications = xjson.getRequiredVector<VerificationView>("verifications");
    verificationState->mRawVerifiedStatus = xjson.getRequiredString("verifiedStatus");
    verificationState->mVerifiedStatus = stringToVerifiedStatus(verificationState->mRawVerifiedStatus);
    verificationState->mRawTrustedVerifierStatus = xjson.getRequiredString("trustedVerifierStatus");
    verificationState->mTrustedVerifierStatus = stringToVerifiedStatus(verificationState->mRawTrustedVerifierStatus);
    return verificationState;
}

QJsonObject KnownFollowers::toJson() const
{
    QJsonObject json;
    json.insert("count", mCount);
    json.insert("followers", XJsonObject::toJsonArray<ProfileViewBasic>(mFollowers));

    return json;
}

KnownFollowers::SharedPtr KnownFollowers::fromJson(const QJsonObject& json)
{
    auto knownFollowers = std::make_shared<KnownFollowers>();
    XJsonObject xjson(json);
    knownFollowers->mCount = xjson.getRequiredInt("count");
    knownFollowers->mFollowers = xjson.getRequiredVector<ProfileViewBasic>("followers");
    return knownFollowers;
}

QJsonObject ViewerState::toJson() const
{
    QJsonObject json;
    XJsonObject::insertOptionalJsonValue(json, "muted", mMuted, false);
    XJsonObject::insertOptionalJsonValue(json, "blockedBy", mBlockedBy, false);
    XJsonObject::insertOptionalJsonValue(json, "blocking", mBlocking);
    XJsonObject::insertOptionalJsonValue(json, "following", mFollowing);
    XJsonObject::insertOptionalJsonValue(json, "followedBy", mFollowedBy);
    XJsonObject::insertOptionalJsonObject<AppBskyGraph::ListViewBasic>(json, "mutedByList", mMutedByList);
    XJsonObject::insertOptionalJsonObject<AppBskyGraph::ListViewBasic>(json, "blockingByList", mBlockingByList);
    XJsonObject::insertOptionalJsonObject<KnownFollowers>(json, "knownFollowers", mKnownFollowers);
    XJsonObject::insertOptionalJsonObject<AppBskyNotification::ActivitySubscription>(json, "activitySubscription", mActivitySubscription);

    return json;
}

ViewerState::SharedPtr ViewerState::fromJson(const QJsonObject& json)
{
    auto viewerState = std::make_shared<ViewerState>();
    XJsonObject xjson(json);
    viewerState->mMuted = xjson.getOptionalBool("muted", false);
    viewerState->mBlockedBy = xjson.getOptionalBool("blockedBy", false);
    viewerState->mBlocking = xjson.getOptionalString("blocking");
    viewerState->mFollowing = xjson.getOptionalString("following");
    viewerState->mFollowedBy = xjson.getOptionalString("followedBy");
    viewerState->mMutedByList = xjson.getOptionalObject<AppBskyGraph::ListViewBasic>("mutedByList");
    viewerState->mBlockingByList = xjson.getOptionalObject<AppBskyGraph::ListViewBasic>("blockingByList");
    viewerState->mKnownFollowers = xjson.getOptionalObject<KnownFollowers>("knownFollowers");
    viewerState->mActivitySubscription = xjson.getOptionalObject<AppBskyNotification::ActivitySubscription>("activitySubscription");
    return viewerState;
}

ActorStatus stringToActorStatus(const QString& str)
{
    static const std::unordered_map<QString, ActorStatus> mapping = {
        { "app.bsky.actor.status#live", ActorStatus::LIVE },
        { "live", ActorStatus::LIVE } // HACK: seems this is used as well??
    };

    return stringToEnum(str, mapping, ActorStatus::UNKNOWN);
}

QString actorStatusToString(ActorStatus status, const QString& unknown)
{
    static const std::unordered_map<ActorStatus, QString> mapping = {
        { ActorStatus::LIVE, "app.bsky.actor.status#live" }
    };

    return enumToString(status, mapping, unknown);
}

QJsonObject StatusView::toJson() const
{
    QJsonObject json;
    json.insert("status", actorStatusToString(mStatus, mRawStatus));
    json.insert("record", mRecord);
    XJsonObject::insertOptionalVariant(json, "embed", mEmbed);
    XJsonObject::insertOptionalDateTime(json, "expiresAt", mExpiresAt);
    XJsonObject::insertOptionalJsonValue(json, "isActive", mIsActive);
    return json;
}

StatusView::SharedPtr StatusView::fromJson(const QJsonObject& json)
{
    auto view = std::make_shared<StatusView>();
    XJsonObject xjson(json);
    view->mRawStatus = xjson.getRequiredString("status");
    view->mStatus = stringToActorStatus(view->mRawStatus);
    view->mRecord = xjson.getRequiredJsonObject("record");
    view->mEmbed = xjson.getOptionalVariant<AppBskyEmbed::ExternalView>("embed");
    view->mExpiresAt = xjson.getOptionalDateTime("expiresAt");
    view->mIsActive = xjson.getOptionalBool("isActive");
    return view;
}

QJsonObject ProfileAssociatedChat::toJson() const
{
    QJsonObject json;
    json.insert("allowIncoming", allowIncomingTypeToString(mAllowIncoming));

    return json;
}

ProfileAssociatedChat::SharedPtr ProfileAssociatedChat::fromJson(const QJsonObject& json)
{
    auto associated = std::make_shared<ProfileAssociatedChat>();
    XJsonObject xjson(json);
    const auto allowIncoming = xjson.getRequiredString("allowIncoming");
    associated->mAllowIncoming = stringToAllowIncomingType(allowIncoming);
    return associated;
}

QJsonObject ProfileAssociatedActivitySubscription::toJson() const
{
    QJsonObject json;
    json.insert("allowSubscriptions", allowSubscriptionsTypeToString(mAllowSubscriptions));

    return json;
}

ProfileAssociatedActivitySubscription::SharedPtr ProfileAssociatedActivitySubscription::fromJson(const QJsonObject& json)
{
    auto associated = std::make_shared<ProfileAssociatedActivitySubscription>();
    XJsonObject xjson(json);
    const auto allowSubscriptions = xjson.getRequiredString("allowSubscriptions");
    associated->mAllowSubscriptions = stringToAllowSubscriptionsType(allowSubscriptions);
    return associated;
}

QJsonObject ProfileAssociated::toJson() const
{
    QJsonObject json;
    XJsonObject::insertOptionalJsonValue<int>(json, "lists", mLists, 0);
    XJsonObject::insertOptionalJsonValue<int>(json, "feedgens", mFeeds, 0);
    XJsonObject::insertOptionalJsonValue<int>(json, "starterPacks", mStarterPacks, 0);
    XJsonObject::insertOptionalJsonValue<bool>(json, "labeler", mLabeler, false);
    XJsonObject::insertOptionalJsonObject<ProfileAssociatedChat>(json, "chat", mChat);
    XJsonObject::insertOptionalJsonObject<ProfileAssociatedActivitySubscription>(json, "activitySubscription", mActivitySubscription);

    return json;
}

ProfileAssociated::SharedPtr ProfileAssociated::fromJson(const QJsonObject& json)
{
    auto associated = std::make_shared<ProfileAssociated>();
    XJsonObject xjson(json);
    associated->mLists = xjson.getOptionalInt("lists", 0);
    associated->mFeeds = xjson.getOptionalInt("feedgens", 0);
    associated->mStarterPacks = xjson.getOptionalInt("starterPacks", 0);
    associated->mLabeler = xjson.getOptionalBool("labeler", false);
    associated->mChat = xjson.getOptionalObject<ProfileAssociatedChat>("chat");
    associated->mActivitySubscription = xjson.getOptionalObject<ProfileAssociatedActivitySubscription>("activitySubscription");

    return associated;
}

QJsonObject ProfileViewBasic::toJson() const
{
    QJsonObject json;
    json.insert("did", mDid);
    json.insert("handle", mHandle);
    XJsonObject::insertOptionalJsonValue(json, "displayName", mDisplayName);
    XJsonObject::insertOptionalJsonValue(json, "avatar", mAvatar);
    XJsonObject::insertOptionalJsonObject<ProfileAssociated>(json, "associated", mAssociated);
    XJsonObject::insertOptionalJsonObject<ViewerState>(json, "viewer", mViewer);
    XJsonObject::insertOptionalArray<ComATProtoLabel::Label>(json, "labels", mLabels);
    XJsonObject::insertOptionalDateTime(json, "createdAt", mCreatedAt);
    XJsonObject::insertOptionalJsonObject<VerificationState>(json, "verification", mVerification);
    XJsonObject::insertOptionalJsonObject<StatusView>(json, "status", mStatus);
    return json;
}

ProfileViewBasic::SharedPtr ProfileViewBasic::fromJson(const QJsonObject& json)
{
    XJsonObject root(json);
    auto profileViewBasic = std::make_shared<ProfileViewBasic>();
    profileViewBasic->mDid = root.getRequiredString("did");
    profileViewBasic->mHandle = root.getRequiredString("handle");
    profileViewBasic->mDisplayName = root.getOptionalString("displayName");
    profileViewBasic->mAvatar = root.getOptionalString("avatar");
    profileViewBasic->mAssociated = root.getOptionalObject<ProfileAssociated>("associated");
    profileViewBasic->mViewer = root.getOptionalObject<ViewerState>("viewer");
    ComATProtoLabel::getLabels(profileViewBasic->mLabels, json);
    profileViewBasic->mCreatedAt = root.getOptionalDateTime("createdAt");
    profileViewBasic->mVerification = root.getOptionalObject<VerificationState>("verification");
    profileViewBasic->mStatus = root.getOptionalObject<StatusView>("status");
    return profileViewBasic;
}

QJsonObject ProfileView::toJson() const
{
    QJsonObject json;
    json.insert("did", mDid);
    json.insert("handle", mHandle);
    XJsonObject::insertOptionalJsonValue(json, "displayName", mDisplayName);
    XJsonObject::insertOptionalJsonValue(json, "avatar", mAvatar);
    XJsonObject::insertOptionalJsonValue(json, "description", mDescription);
    return json;
}

ProfileView::SharedPtr ProfileView::fromJson(const QJsonObject& json)
{
    XJsonObject root(json);
    auto profile = std::make_shared<ProfileView>();
    profile->mDid = root.getRequiredString("did");
    profile->mHandle = root.getRequiredString("handle");
    profile->mDisplayName = root.getOptionalString("displayName");
    profile->mAvatar = root.getOptionalString("avatar");
    profile->mAssociated = root.getOptionalObject<ProfileAssociated>("associated");
    profile->mDescription = root.getOptionalString("description");
    profile->mIndexedAt = root.getOptionalDateTime("indexedAt");
    profile->mCreatedAt = root.getOptionalDateTime("createdAt");
    profile->mViewer = root.getOptionalObject<ViewerState>("viewer");
    ComATProtoLabel::getLabels(profile->mLabels, json);
    profile->mVerification = root.getOptionalObject<VerificationState>("verification");
    profile->mStatus = root.getOptionalObject<StatusView>("status");
    return profile;
}

ProfileViewDetailed::SharedPtr ProfileViewDetailed::fromJson(const QJsonObject& json)
{
    XJsonObject root(json);
    auto profile = std::make_shared<ProfileViewDetailed>();
    profile->mDid = root.getRequiredString("did");
    profile->mHandle = root.getRequiredString("handle");
    profile->mDisplayName = root.getOptionalString("displayName");
    profile->mAvatar = root.getOptionalString("avatar");
    profile->mBanner = root.getOptionalString("banner");
    profile->mDescription = root.getOptionalString("description");
    profile->mFollowersCount = root.getOptionalInt("followersCount", 0);
    profile->mFollowsCount = root.getOptionalInt("followsCount", 0);
    profile->mPostsCount = root.getOptionalInt("postsCount", 0);
    profile->mAssociated = root.getOptionalObject<ProfileAssociated>("associated");
    profile->mIndexedAt = root.getOptionalDateTime("indexedAt");
    profile->mCreatedAt = root.getOptionalDateTime("createdAt");
    profile->mViewer = root.getOptionalObject<ViewerState>("viewer");
    ComATProtoLabel::getLabels(profile->mLabels, json);
    profile->mPinnedPost = root.getOptionalObject<ComATProtoRepo::StrongRef>("pinnedPost");
    profile->mVerification = root.getOptionalObject<VerificationState>("verification");
    profile->mStatus = root.getOptionalObject<StatusView>("status");
    return profile;
}

GetProfilesOutput::SharedPtr GetProfilesOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<GetProfilesOutput>();
    output->mProfiles = xjson.getRequiredVector<ProfileViewDetailed>("profiles");
    return output;
}

QJsonObject Profile::toJson() const
{
    QJsonObject json(mJson);
    XJsonObject::insertOptionalJsonValue(json, "displayName", mDisplayName);
    XJsonObject::insertOptionalJsonValue(json, "description", mDescription);
    XJsonObject::insertOptionalJsonObject<Blob>(json, "avatar", mAvatar);
    XJsonObject::insertOptionalJsonObject<Blob>(json, "banner", mBanner);
    XJsonObject::insertOptionalJsonObject<ComATProtoLabel::SelfLabels>(json, "labels", mLabels);
    XJsonObject::insertOptionalJsonObject<ComATProtoRepo::StrongRef>(json, "pinnedPost", mPinndedPost);
    return json;
}

Profile::SharedPtr Profile::fromJson(const QJsonObject& json)
{
    auto profile = std::make_shared<Profile>();
    XJsonObject xjson(json);
    profile->mJson = json;
    profile->mDisplayName = xjson.getOptionalString("displayName");
    profile->mDescription = xjson.getOptionalString("description");
    profile->mAvatar = xjson.getOptionalObject<Blob>("avatar");
    profile->mBanner = xjson.getOptionalObject<Blob>("banner");
    profile->mLabels = xjson.getOptionalObject<ComATProtoLabel::SelfLabels>("labels");
    profile->mPinndedPost = xjson.getOptionalObject<ComATProtoRepo::StrongRef>("pinnedPost");
    return profile;
}

QJsonObject AdultContentPref::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", "app.bsky.actor.defs#adultContentPref");
    json.insert("enabled", mEnabled);
    return json;
}

AdultContentPref::SharedPtr AdultContentPref::fromJson(const QJsonObject& json)
{
    auto adultPref = std::make_shared<AdultContentPref>();
    XJsonObject xjson(json);
    adultPref->mEnabled = xjson.getRequiredBool("enabled");
    adultPref->mJson = json;
    return adultPref;
}

ContentLabelPref::Visibility ContentLabelPref::stringToVisibility(const QString& str)
{
    static const std::unordered_map<QString, Visibility> mapping = {
        { "show", Visibility::SHOW },
        { "ignore", Visibility::SHOW },
        { "warn", Visibility::WARN },
        { "hide", Visibility::HIDE }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    qWarning() << "Unknown content label pref visibility:" << str;
    return Visibility::UNKNOWN;
}

QString ContentLabelPref::visibilityToString(Visibility visibility, const QString& unknown)
{
    static const std::unordered_map<Visibility, QString> mapping = {
        { Visibility::SHOW, "ignore" },
        { Visibility::WARN, "warn" },
        { Visibility::HIDE, "hide" }
    };

    const auto it = mapping.find(visibility);
    Q_ASSERT(it != mapping.end());

    if (it == mapping.end())
    {
        qWarning() << "Unknown visibility:" << int(visibility);
        return unknown;
    }

    return it->second;
}

QJsonObject ContentLabelPref::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", "app.bsky.actor.defs#contentLabelPref");
    XJsonObject::insertOptionalJsonValue(json, "labelerDid", mLabelerDid);
    json.insert("label", mLabel);
    json.insert("visibility", visibilityToString(mVisibility, mRawVisibility));
    return json;
}

ContentLabelPref::SharedPtr ContentLabelPref::fromJson(const QJsonObject& json)
{
    auto pref = std::make_shared<ContentLabelPref>();
    XJsonObject xjson(json);
    pref->mLabelerDid = xjson.getOptionalString("labelerDid");
    pref->mLabel = xjson.getRequiredString("label");
    pref->mRawVisibility = xjson.getRequiredString("visibility");
    pref->mVisibility = stringToVisibility(pref->mRawVisibility);
    pref->mJson = json;
    return pref;
}

QJsonObject SavedFeedsPref::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", "app.bsky.actor.defs#savedFeedsPref");
    json.insert("pinned", XJsonObject::toJsonArray(mPinned));
    json.insert("saved", XJsonObject::toJsonArray(mSaved));
    return json;
}

SavedFeedsPref::SharedPtr SavedFeedsPref::fromJson(const QJsonObject& json)
{
    auto pref = std::make_shared<SavedFeedsPref>();
    XJsonObject xjson(json);
    pref->mPinned = xjson.getRequiredStringVector("pinned");
    pref->mSaved = xjson.getRequiredStringVector("saved");
    pref->mJson = json;
    return pref;
}

PersonalDetailsPref::SharedPtr PersonalDetailsPref::fromJson(const QJsonObject& json)
{
    auto pref = std::make_shared<PersonalDetailsPref>();
    XJsonObject xjson(json);
    pref->mBirthDate = xjson.getOptionalDateTime("birthDate");
    pref->mJson = json;
    return pref;
}

QJsonObject FeedViewPref::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", "app.bsky.actor.defs#feedViewPref");
    json.insert("feed", mFeed);
    json.insert("hideReplies", mHideReplies);
    json.insert("hideRepliesByUnfollowed", mHideRepliesByUnfollowed);
    json.insert("hideRepliesByLikeCount", mHideRepliesByLikeCount);
    json.insert("hideReposts", mHideReposts);
    json.insert("hideQuotePosts", mHideQuotePosts);
    return json;
}

FeedViewPref::SharedPtr FeedViewPref::fromJson(const QJsonObject& json)
{
    auto pref = std::make_shared<FeedViewPref>();
    XJsonObject xjson(json);
    pref->mFeed = xjson.getRequiredString("feed");
    pref->mHideReplies = xjson.getOptionalBool("hideReplies", false);
    pref->mHideRepliesByUnfollowed = xjson.getOptionalBool("hideRepliesByUnfollowed", true);
    pref->mHideRepliesByLikeCount = xjson.getOptionalInt("hideRepliesByLikeCount", 0);
    pref->mHideReposts = xjson.getOptionalBool("hideReposts", false);
    pref->mHideQuotePosts = xjson.getOptionalBool("hideQuotePosts", false);
    pref->mJson = json;
    return pref;
}

ThreadViewPref::SharedPtr ThreadViewPref::fromJson(const QJsonObject& json)
{
    auto pref = std::make_shared<ThreadViewPref>();
    XJsonObject xjson(json);
    pref->mSort = xjson.getOptionalString("sort");
    pref->mPrioritizeFollowedUsers = xjson.getOptionalBool("prioritizeFollowedUsers", false);
    pref->mJson = json;
    return pref;
}

MutedWordTarget stringToMutedWordTarget(const QString& str)
{
    static const std::unordered_map<QString, MutedWordTarget> mapping = {
        { "content", MutedWordTarget::CONTENT },
        { "tag", MutedWordTarget::TAG }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    qDebug() << "Unknown muted word target:" << str;
    return MutedWordTarget::UNKNOWN;
}

QString mutedWordTargetToString(MutedWordTarget target)
{
    static const std::unordered_map<MutedWordTarget, QString> mapping = {
        { MutedWordTarget::CONTENT, "content" },
        { MutedWordTarget::TAG, "tag" }
    };

    const auto it = mapping.find(target);
    if (it != mapping.end())
        return it->second;

    qDebug() << "Unknown muted word target:" << (int)target;
    return {};
}

ActorTarget stringToActorTarget(const QString& str)
{
    static const std::unordered_map<QString, ActorTarget> mapping = {
        { "all", ActorTarget::ALL },
        { "exclude-following", ActorTarget::EXCLUDE_FOLLOWING }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    qDebug() << "Unknown actor target:" << str;
    return ActorTarget::UNKNOWN;
}

QString actorTargetToString(ActorTarget target)
{
    static const std::unordered_map<ActorTarget, QString> mapping = {
        { ActorTarget::ALL, "all" },
        { ActorTarget::EXCLUDE_FOLLOWING, "exclude-following" }
    };

    const auto it = mapping.find(target);
    if (it != mapping.end())
        return it->second;

    qDebug() << "Unknown actor target:" << (int)target;
    return {};
}

QJsonObject MutedWord::toJson() const
{
    QJsonObject json(mJson);
    json.insert("value", mValue);
    std::vector<QString> targets;

    for (const auto& target : mTargets)
    {
        const auto tgtString = mutedWordTargetToString(target.mTarget);
        targets.push_back(!tgtString.isEmpty() ? tgtString : target.mRawTarget);
    }

    json.insert("targets", XJsonObject::toJsonArray(targets));

    if (mActorTarget != ActorTarget::ALL)
    {
        const auto actorTgtString = actorTargetToString(mActorTarget);
        json.insert("actorTarget", !actorTgtString.isEmpty() ? actorTgtString : mRawActorTarget);
    }
    else
    {
        json.remove("actorTarget");
    }

    XJsonObject::insertOptionalDateTime(json, "expiresAt", mExpiresAt);
    return json;
}

MutedWord::SharedPtr MutedWord::fromJson(const QJsonObject& json)
{
    auto mutedWord = std::make_shared<MutedWord>();
    const XJsonObject xjson(json);
    mutedWord->mValue = xjson.getRequiredString("value");
    const auto targets = xjson.getRequiredStringVector("targets");

    for (const auto& targetString : targets)
    {
        Target target;
        target.mTarget = stringToMutedWordTarget(targetString);
        target.mRawTarget = targetString;
        mutedWord->mTargets.push_back(target);
    }

    mutedWord->mRawActorTarget = xjson.getOptionalString("actorTarget", mutedWord->mRawActorTarget);
    mutedWord->mActorTarget = stringToActorTarget(mutedWord->mRawActorTarget);
    mutedWord->mExpiresAt = xjson.getOptionalDateTime("expiresAt");

    mutedWord->mJson = json;
    return mutedWord;
}

QJsonObject MutedWordsPref::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", "app.bsky.actor.defs#mutedWordsPref");
    QJsonArray jsonArray;

    for (const auto& item : mItems)
        jsonArray.push_back(item.toJson());

    json.insert("items", jsonArray);
    return json;
}

MutedWordsPref::SharedPtr MutedWordsPref::fromJson(const QJsonObject& json)
{
    auto pref = std::make_shared<MutedWordsPref>();
    const XJsonObject xjson(json);
    auto items = xjson.getRequiredVector<MutedWord>("items");
    pref->mItems.reserve(items.size());

    for (auto& item : items)
        pref->mItems.push_back(std::move(*item));

    pref->mJson = json;
    return pref;
}

QJsonObject LabelerPrefItem::toJson() const
{
    QJsonObject json(mJson);
    json.insert("did", mDid);
    return json;
}

LabelerPrefItem::SharedPtr LabelerPrefItem::fromJson(const QJsonObject& json)
{
    auto item = std::make_shared<LabelerPrefItem>();
    const XJsonObject xjson(json);
    item->mDid = xjson.getRequiredString("did");
    item->mJson = json;
    return item;
}

QJsonObject LabelersPref::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", "app.bsky.actor.defs#labelersPref");

    QJsonArray jsonArray;

    for (const auto& labeler : mLabelers)
        jsonArray.push_back(labeler.toJson());

    json.insert("labelers", jsonArray);
    return json;
}

LabelersPref::SharedPtr LabelersPref::fromJson(const QJsonObject& json)
{
    auto pref = std::make_shared<LabelersPref>();
    const XJsonObject xjson(json);
    auto labelers = xjson.getOptionalVector<LabelerPrefItem>("labelers");

    for (auto& labeler : labelers)
        pref->mLabelers.insert(std::move(*labeler));

    pref->mJson = json;
    return pref;
}

QJsonObject PostInteractionSettingsPref::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", "app.bsky.actor.defs#postInteractionSettingsPref");

    QJsonArray allowArray = mRules.toJson();

    if (!allowArray.isEmpty() || mRules.mAllowNobody)
        json.insert("threadgateAllowRules", allowArray);

    AppBskyFeed::PostgateEmbeddingRules::insertDisableEmbedding(json, "postgateEmbeddingRules", mDisableEmbedding);
    return json;
}

PostInteractionSettingsPref::SharedPtr PostInteractionSettingsPref::fromJson(const QJsonObject& json)
{
    auto pref = std::make_shared<PostInteractionSettingsPref>();
    const XJsonObject xjson(json);
    const auto allowArray = xjson.getOptionalArray("threadgateAllowRules");

    if (allowArray)
    {
        const auto rules = AppBskyFeed::ThreadgateRules::fromJson(*allowArray);
        pref->mRules = *rules;
        pref->mRules.mAllowNobody = allowArray->empty();
    }

    pref->mDisableEmbedding = AppBskyFeed::PostgateEmbeddingRules::getDisableEmbedding(json, "postgateEmbeddingRules");
    pref->mJson = json;
    return pref;
}

QJsonObject VerificationPrefs::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", TYPE);
    json.insert("hideBadges", mHideBadges);
    return json;
}

VerificationPrefs::SharedPtr VerificationPrefs::fromJson(const QJsonObject& json)
{
    auto pref = std::make_shared<VerificationPrefs>();
    const XJsonObject xjson(json);
    pref->mHideBadges = xjson.getOptionalBool("hideBadges", false);
    return pref;
}

UnknownPref::SharedPtr UnknownPref::fromJson(const QJsonObject& json)
{
    auto pref = std::make_shared<UnknownPref>();
    pref->mJson = json;
    return pref;
}

PreferenceType stringToPreferenceType(const QString& str)
{
    static const std::unordered_map<QString, PreferenceType> mapping = {
        { "app.bsky.actor.defs#adultContentPref", PreferenceType::ADULT_CONTENT },
        { "app.bsky.actor.defs#contentLabelPref", PreferenceType::CONTENT_LABEL },
        { "app.bsky.actor.defs#savedFeedsPref", PreferenceType::SAVED_FEEDS },
        { "app.bsky.actor.defs#personalDetailsPref", PreferenceType::PERSONAL_DETAILS },
        { "app.bsky.actor.defs#feedViewPref", PreferenceType::FEED_VIEW },
        { "app.bsky.actor.defs#threadViewPref", PreferenceType::THREAD_VIEW },
        { "app.bsky.actor.defs#mutedWordsPref", PreferenceType::MUTED_WORDS },
        { "app.bsky.actor.defs#labelersPref", PreferenceType::LABELERS },
        { "app.bsky.actor.defs#postInteractionSettingsPref", PreferenceType::POST_INTERACTION_SETTINGS },
        { VerificationPrefs::TYPE, PreferenceType::VERIFICATION }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    qDebug() << "Unknown preference type:" << str;
    return PreferenceType::UNKNOWN;
}

Preference::SharedPtr Preference::fromJson(const QJsonObject& json)
{
    auto pref = std::make_shared<Preference>();
    const XJsonObject xjson(json);
    pref->mRawType = xjson.getRequiredString("$type");
    pref->mType = stringToPreferenceType(pref->mRawType);

    switch (pref->mType) {
    case PreferenceType::ADULT_CONTENT:
        pref->mItem = AdultContentPref::fromJson(json);
        break;
    case PreferenceType::CONTENT_LABEL:
        pref->mItem = ContentLabelPref::fromJson(json);
        break;
    case PreferenceType::SAVED_FEEDS:
        pref->mItem = SavedFeedsPref::fromJson(json);
        break;
    case PreferenceType::PERSONAL_DETAILS:
        pref->mItem = PersonalDetailsPref::fromJson(json);
        break;
    case PreferenceType::FEED_VIEW:
        pref->mItem = FeedViewPref::fromJson(json);
        break;
    case PreferenceType::THREAD_VIEW:
        pref->mItem = ThreadViewPref::fromJson(json);
        break;
    case PreferenceType::MUTED_WORDS:
        pref->mItem = MutedWordsPref::fromJson(json);
        break;
    case PreferenceType::LABELERS:
        pref->mItem = LabelersPref::fromJson(json);
        break;
    case PreferenceType::POST_INTERACTION_SETTINGS:
        pref->mItem = PostInteractionSettingsPref::fromJson(json);
        break;
    case PreferenceType::VERIFICATION:
        pref->mItem = VerificationPrefs::fromJson(json);
        break;
    case PreferenceType::UNKNOWN:
        pref->mItem = UnknownPref::fromJson(json);
        break;
    }

    return pref;
}

QJsonObject GetPreferencesOutput::toJson() const
{
    QJsonArray jsonArray;

    for (const auto& pref : mPreferences)
    {
        QJsonObject prefJson;
        std::visit([&prefJson](auto&& x){ prefJson = x->toJson(); }, pref->mItem);

        if (!prefJson.empty())
            jsonArray.append(prefJson);
    }

    QJsonObject json;
    json.insert("preferences", jsonArray);
    return json;
}

GetPreferencesOutput::SharedPtr GetPreferencesOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetPreferencesOutput>();
    const XJsonObject xjson(json);
    output->mPreferences = xjson.getRequiredVector<Preference>("preferences");
    return output;
}

SearchActorsOutput::SharedPtr SearchActorsOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<SearchActorsOutput>();
    const XJsonObject xjson(json);
    output->mCursor = xjson.getOptionalString("cursor");
    output->mActors = xjson.getRequiredVector<ProfileView>("actors");
    return output;
}

SearchActorsTypeaheadOutput::SharedPtr SearchActorsTypeaheadOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<SearchActorsTypeaheadOutput>();
    const XJsonObject xjson(json);
    output->mActors = xjson.getRequiredVector<ProfileViewBasic>("actors");
    return output;
}

GetSuggestionsOutput::SharedPtr GetSuggestionsOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetSuggestionsOutput>();
    const XJsonObject xjson(json);
    output->mCursor = xjson.getOptionalString("cursor");
    output->mActors = xjson.getRequiredVector<ProfileView>("actors");
    return output;
}

GetSuggestedFollowsByActor::SharedPtr GetSuggestedFollowsByActor::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetSuggestedFollowsByActor>();
    const XJsonObject xjson(json);
    output->mSuggestions = xjson.getRequiredVector<ProfileView>("suggestions");
    return output;
}

}
