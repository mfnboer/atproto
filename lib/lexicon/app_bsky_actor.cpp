// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_actor.h"
#include "app_bsky_graph.h"
#include "../xjson.h"
#include <unordered_map>

namespace ATProto::AppBskyActor {

ViewerState::Ptr ViewerState::fromJson(const QJsonObject& json)
{
    auto viewerState = std::make_unique<ViewerState>();
    XJsonObject xjson(json);
    viewerState->mMuted = xjson.getOptionalBool("muted", false);
    viewerState->mBlockedBy = xjson.getOptionalBool("blockedBy", false);
    viewerState->mBlocking = xjson.getOptionalString("blocking");
    viewerState->mFollowing = xjson.getOptionalString("following");
    viewerState->mFollowedBy = xjson.getOptionalString("followedBy");
    viewerState->mMutedByList = xjson.getOptionalObject<AppBskyGraph::ListViewBasic>("mutedByList");
    viewerState->mBlockingByList = xjson.getOptionalObject<AppBskyGraph::ListViewBasic>("blockingByList");
    return viewerState;
}

ProfileAssociated::Ptr ProfileAssociated::fromJson(const QJsonObject& json)
{
    auto associated = std::make_unique<ProfileAssociated>();
    XJsonObject xjson(json);
    associated->mLists = xjson.getOptionalInt("lists", 0);
    associated->mFeeds = xjson.getOptionalInt("feeds", 0);
    associated->mLabeler = xjson.getOptionalBool("labeler", false);
    return associated;
}

QJsonObject ProfileViewBasic::toJson() const
{
    QJsonObject json;
    json.insert("did", mDid);
    json.insert("handle", mHandle);
    XJsonObject::insertOptionalJsonValue(json, "displayName", mDisplayName);
    XJsonObject::insertOptionalJsonValue(json, "avatar", mAvatar);
    return json;
}

ProfileViewBasic::Ptr ProfileViewBasic::fromJson(const QJsonObject& json)
{
    XJsonObject root(json);
    auto profileViewBasic = std::make_unique<ProfileViewBasic>();
    profileViewBasic->mDid = root.getRequiredString("did");
    profileViewBasic->mHandle = root.getRequiredString("handle");
    profileViewBasic->mDisplayName = root.getOptionalString("displayName");
    profileViewBasic->mAvatar = root.getOptionalString("avatar");
    profileViewBasic->mAssociated = root.getOptionalObject<ProfileAssociated>("associated");
    profileViewBasic->mViewer = root.getOptionalObject<ViewerState>("viewer");
    ComATProtoLabel::getLabels(profileViewBasic->mLabels, json);
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

ProfileView::Ptr ProfileView::fromJson(const QJsonObject& json)
{
    XJsonObject root(json);
    auto profile = std::make_unique<ProfileView>();
    profile->mDid = root.getRequiredString("did");
    profile->mHandle = root.getRequiredString("handle");
    profile->mDisplayName = root.getOptionalString("displayName");
    profile->mAvatar = root.getOptionalString("avatar");
    profile->mAssociated = root.getOptionalObject<ProfileAssociated>("associated");
    profile->mDescription = root.getOptionalString("description");
    profile->mIndexedAt = root.getOptionalDateTime("indexedAt");
    profile->mViewer = root.getOptionalObject<ViewerState>("viewer");
    ComATProtoLabel::getLabels(profile->mLabels, json);
    return profile;
}

ProfileViewDetailed::Ptr ProfileViewDetailed::fromJson(const QJsonObject& json)
{
    XJsonObject root(json);
    auto profile = std::make_unique<ProfileViewDetailed>();
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
    profile->mViewer = root.getOptionalObject<ViewerState>("viewer");
    ComATProtoLabel::getLabels(profile->mLabels, json);
    return profile;
}

void getProfileViewDetailedList(ProfileViewDetailedList& list, const QJsonObject& json)
{
    XJsonObject xjson(json);
    list = xjson.getRequiredVector<ProfileViewDetailed>("profiles");
}

QJsonObject Profile::toJson() const
{
    QJsonObject json(mJson);
    XJsonObject::insertOptionalJsonValue(json, "displayName", mDisplayName);
    XJsonObject::insertOptionalJsonValue(json, "description", mDescription);
    XJsonObject::insertOptionalJsonObject<Blob>(json, "avatar", mAvatar);
    XJsonObject::insertOptionalJsonObject<Blob>(json, "banner", mBanner);
    XJsonObject::insertOptionalJsonObject<ComATProtoLabel::SelfLabels>(json, "labels", mLabels);
    return json;
}

Profile::Ptr Profile::fromJson(const QJsonObject& json)
{
    auto profile = std::make_unique<Profile>();
    XJsonObject xjson(json);
    profile->mJson = json;
    profile->mDisplayName = xjson.getOptionalString("displayName");
    profile->mDescription = xjson.getOptionalString("description");
    profile->mAvatar = xjson.getOptionalObject<Blob>("avatar");
    profile->mBanner = xjson.getOptionalObject<Blob>("banner");
    profile->mLabels = xjson.getOptionalObject<ComATProtoLabel::SelfLabels>("labels");
    return profile;
}

QJsonObject AdultContentPref::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", "app.bsky.actor.defs#adultContentPref");
    json.insert("enabled", mEnabled);
    return json;
}

AdultContentPref::Ptr AdultContentPref::fromJson(const QJsonObject& json)
{
    auto adultPref = std::make_unique<AdultContentPref>();
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
        { Visibility::SHOW, "show" },
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

    if (!isGlobal() && mVisibility == Visibility::SHOW)
        json.insert("visibility", "ignore");
    else
        json.insert("visibility", visibilityToString(mVisibility, mRawVisibility));

    return json;
}

ContentLabelPref::Ptr ContentLabelPref::fromJson(const QJsonObject& json)
{
    auto pref = std::make_unique<ContentLabelPref>();
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

SavedFeedsPref::Ptr SavedFeedsPref::fromJson(const QJsonObject& json)
{
    auto pref = std::make_unique<SavedFeedsPref>();
    XJsonObject xjson(json);
    pref->mPinned = xjson.getRequiredStringVector("pinned");
    pref->mSaved = xjson.getRequiredStringVector("saved");
    pref->mJson = json;
    return pref;
}

PersonalDetailsPref::Ptr PersonalDetailsPref::fromJson(const QJsonObject& json)
{
    auto pref = std::make_unique<PersonalDetailsPref>();
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

FeedViewPref::Ptr FeedViewPref::fromJson(const QJsonObject& json)
{
    auto pref = std::make_unique<FeedViewPref>();
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

ThreadViewPref::Ptr ThreadViewPref::fromJson(const QJsonObject& json)
{
    auto pref = std::make_unique<ThreadViewPref>();
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
    return json;
}

MutedWord::Ptr MutedWord::fromJson(const QJsonObject& json)
{
    auto mutedWord = std::make_unique<MutedWord>();
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

MutedWordsPref::Ptr MutedWordsPref::fromJson(const QJsonObject& json)
{
    auto pref = std::make_unique<MutedWordsPref>();
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

LabelerPrefItem::Ptr LabelerPrefItem::fromJson(const QJsonObject& json)
{
    auto item = std::make_unique<LabelerPrefItem>();
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

    return json;
}

LabelersPref::Ptr LabelersPref::fromJson(const QJsonObject& json)
{
    auto pref = std::make_unique<LabelersPref>();
    const XJsonObject xjson(json);
    auto labelers = xjson.getRequiredVector<LabelerPrefItem>("labelers");
    pref->mLabelers.reserve(labelers.size());

    for (auto& labeler : labelers)
        pref->mLabelers.push_back(std::move(*labeler));

    pref->mJson = json;
    return pref;
}

UnknownPref::Ptr UnknownPref::fromJson(const QJsonObject& json)
{
    auto pref = std::make_unique<UnknownPref>();
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
        { "app.bsky.actor.defs#labelersPref", PreferenceType::LABELERS }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    qDebug() << "Unknown preference type:" << str;
    return PreferenceType::UNKNOWN;
}

Preference::Ptr Preference::fromJson(const QJsonObject& json)
{
    auto pref = std::make_unique<Preference>();
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

GetPreferencesOutput::Ptr GetPreferencesOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_unique<GetPreferencesOutput>();
    const XJsonObject xjson(json);
    output->mPreferences = xjson.getRequiredVector<Preference>("preferences");
    return output;
}

SearchActorsOutput::Ptr SearchActorsOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_unique<SearchActorsOutput>();
    const XJsonObject xjson(json);
    output->mCursor = xjson.getOptionalString("cursor");
    output->mActors = xjson.getRequiredVector<ProfileView>("actors");
    return output;
}

SearchActorsTypeaheadOutput::Ptr SearchActorsTypeaheadOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_unique<SearchActorsTypeaheadOutput>();
    const XJsonObject xjson(json);
    output->mActors = xjson.getRequiredVector<ProfileViewBasic>("actors");
    return output;
}

GetSuggestionsOutput::Ptr GetSuggestionsOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_unique<GetSuggestionsOutput>();
    const XJsonObject xjson(json);
    output->mCursor = xjson.getOptionalString("cursor");
    output->mActors = xjson.getRequiredVector<ProfileView>("actors");
    return output;
}

}
