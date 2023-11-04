// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_actor.h"
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
    return viewerState;
}

ProfileViewBasic::Ptr ProfileViewBasic::fromJson(const QJsonObject& json)
{
    XJsonObject root(json);
    auto profileViewBasic = std::make_unique<ProfileViewBasic>();
    profileViewBasic->mDid = root.getRequiredString("did");
    profileViewBasic->mHandle = root.getRequiredString("handle");
    profileViewBasic->mDisplayName = root.getOptionalString("displayName");
    profileViewBasic->mAvatar = root.getOptionalString("avatar");

    const auto viewerJson = root.getOptionalObject("viewer");
    if (viewerJson)
        profileViewBasic->mViewer = ViewerState::fromJson(*viewerJson);

    ComATProtoLabel::getLabels(profileViewBasic->mLabels, json);
    return profileViewBasic;
}

ProfileView::Ptr ProfileView::fromJson(const QJsonObject& json)
{
    XJsonObject root(json);
    auto profile = std::make_unique<ProfileView>();
    profile->mDid = root.getRequiredString("did");
    profile->mHandle = root.getRequiredString("handle");
    profile->mDisplayName = root.getOptionalString("displayName");
    profile->mAvatar = root.getOptionalString("avatar");
    profile->mDescription = root.getOptionalString("description");
    profile->mIndexedAt = root.getOptionalDateTime("indexedAt");

    const auto viewerJson = root.getOptionalObject("viewer");
    if (viewerJson)
        profile->mViewer = ViewerState::fromJson(*viewerJson);

    ComATProtoLabel::getLabels(profile->mLabels, json);
    return profile;
}

void getProfileViewList(ProfileViewList& list, const QJsonObject& json, const QString& fieldName)
{   
    XJsonObject xjson(json);
    list = xjson.getRequiredVector<ProfileView>(fieldName);
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
    profile->mIndexedAt = root.getOptionalDateTime("indexedAt");

    const auto viewerJson = root.getOptionalObject("viewer");
    if (viewerJson)
        profile->mViewer = ViewerState::fromJson(*viewerJson);

    ComATProtoLabel::getLabels(profile->mLabels, json);
    return profile;
}

void getProfileViewDetailedList(ProfileViewDetailedList& list, const QJsonObject& json)
{
    XJsonObject xjson(json);
    list = xjson.getRequiredVector<ProfileViewDetailed>("profiles");
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
        { "ignore", Visibility::SHOW }, // not in spec, but I get this from bluesky
        { "warn", Visibility::WARN },
        { "hide", Visibility::HIDE }
    };

    const auto it = mapping.find(str);
    if (it != mapping.end())
        return it->second;

    qWarning() << "Unknown content label pref visibility:" << str;
    return Visibility::UNKNOWN;
}

QString ContentLabelPref::visibilityToString(Visibility visibility)
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
        return "hide";
    }

    return it->second;
}

QJsonObject ContentLabelPref::toJson() const
{
    QJsonObject json(mJson);
    json.insert("$type", "app.bsky.actor.defs#contentLabelPref");
    json.insert("label", mLabel);
    json.insert("visibility", visibilityToString(mVisibility));
    return json;
}

ContentLabelPref::Ptr ContentLabelPref::fromJson(const QJsonObject& json)
{
    auto pref = std::make_unique<ContentLabelPref>();
    XJsonObject xjson(json);
    pref->mLabel = xjson.getRequiredString("label");
    pref->mRawVisibility = xjson.getRequiredString("visibility");
    pref->mVisibility = stringToVisibility(pref->mRawVisibility);
    pref->mJson = json;
    return pref;
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
        { "app.bsky.actor.defs#threadViewPref", PreferenceType::THREAD_VIEW }
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

LegacySearchActorsOutput::Ptr LegacySearchActorsOutput::fromJson(const QJsonArray& jsonArray)
{
    auto output = std::make_unique<LegacySearchActorsOutput>();
    output->mDids.reserve(jsonArray.size());

    for (const auto& profileJsonEntry : jsonArray)
    {
        if (!profileJsonEntry.isObject())
        {
            qWarning() << "Invalid profile:" << jsonArray << profileJsonEntry;
            throw InvalidJsonException("LegacySearchActorsOutput");
        }

        const QJsonObject profileJson = profileJsonEntry.toObject();
        const XJsonObject xjsonProfile(profileJson);
        const QString did = xjsonProfile.getRequiredString("did");
        output->mDids.push_back(did);
    }

    return output;
}

}
