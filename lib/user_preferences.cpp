// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "user_preferences.h"

namespace ATProto {

UserPreferences::UserPreferences(const AppBskyActor::PreferenceList& preferences)
{
    setPrefs(preferences);
}

void UserPreferences::setPrefs(const AppBskyActor::PreferenceList& preferences)
{
    for (const auto& pref : preferences)
    {
        if (holdsNonNull<AppBskyActor::AdultContentPref::SharedPtr>(pref))
        {
            const auto& adultContent = std::get<AppBskyActor::AdultContentPref::SharedPtr>(pref);
            mAdultContent = adultContent->mEnabled;
        }
        else if (holdsNonNull<AppBskyActor::ContentLabelPref::SharedPtr>(pref))
        {
            const auto& contentLabel = std::get<AppBskyActor::ContentLabelPref::SharedPtr>(pref);

            if (contentLabel->mVisibility != LabelVisibility::UNKNOWN)
            {
                const QString did = contentLabel->mLabelerDid.value_or("");
                mContentLabelPrefs[did][contentLabel->mLabel] = contentLabel->mVisibility;
            }
        }
        else if (holdsNonNull<AppBskyActor::SavedFeedsPref::SharedPtr>(pref))
        {
            const auto& savedFeed = std::get<AppBskyActor::SavedFeedsPref::SharedPtr>(pref);
            mSavedFeedsPref = *savedFeed;
        }
        else if (holdsNonNull<AppBskyActor::SavedFeedsPrefV2::SharedPtr>(pref))
        {
            const auto& savedFeedsV2 = std::get<AppBskyActor::SavedFeedsPrefV2::SharedPtr>(pref);
            mSavedFeedsPrefV2 = *savedFeedsV2;
        }
        else if (holdsNonNull<AppBskyActor::PersonalDetailsPref::SharedPtr>(pref))
        {
            const auto& personal = std::get<AppBskyActor::PersonalDetailsPref::SharedPtr>(pref);
            mPersonalDetailsPref = *personal;
            mBirthDate = personal->mBirthDate;
        }
        else if (holdsNonNull<AppBskyActor::FeedViewPref::SharedPtr>(pref))
        {
            const auto& feedView = std::get<AppBskyActor::FeedViewPref::SharedPtr>(pref);
            mFeedViewPrefs[feedView->mFeed] = *feedView;
        }
        else if (holdsNonNull<AppBskyActor::ThreadViewPref::SharedPtr>(pref))
        {
            const auto& threadView = std::get<AppBskyActor::ThreadViewPref::SharedPtr>(pref);
            mThreadViewPref = *threadView;
        }
        else if (holdsNonNull<AppBskyActor::MutedWordsPref::SharedPtr>(pref))
        {
            const auto& mutedWords = std::get<AppBskyActor::MutedWordsPref::SharedPtr>(pref);
            mMutedWordsPref = *mutedWords;
        }
        else if (holdsNonNull<AppBskyActor::LabelersPref::SharedPtr>(pref))
        {
            const auto& labelers = std::get<AppBskyActor::LabelersPref::SharedPtr>(pref);
            mLabelersPref = *labelers;
        }
        else if (holdsNonNull<AppBskyActor::PostInteractionSettingsPref::SharedPtr>(pref))
        {
            const auto& postInteractionSettings = std::get<AppBskyActor::PostInteractionSettingsPref::SharedPtr>(pref);
            mPostInteractionSettingsPref = *postInteractionSettings;
        }
        else if (holdsNonNull<AppBskyActor::VerificationPrefs::SharedPtr>(pref))
        {
            const auto& verification = std::get<AppBskyActor::VerificationPrefs::SharedPtr>(pref);
            mVerificationPrefs = *verification;
        }
        else if (holdsNonNull<AppBskyActor::UnknownPref::SharedPtr>(pref))
        {
            const auto& unknowPref = std::get<AppBskyActor::UnknownPref::SharedPtr>(pref);
            mUnknownPrefs.push_back(*unknowPref);
        }
    }
}

std::unordered_set<QString> UserPreferences::getLabelerDids() const
{
    std::unordered_set<QString> dids;

    for (const auto& item : mLabelersPref.mLabelers)
        dids.insert(item.mDid);

    return dids;
}

AppBskyActor::PreferenceList UserPreferences::toPreferenceList() const
{
    AppBskyActor::PreferenceItem pref;
    AppBskyActor::PreferenceList preferences;

    auto adultContentPref = std::make_shared<AppBskyActor::AdultContentPref>();
    adultContentPref->mEnabled = mAdultContent;
    preferences.push_back(std::move(adultContentPref));

    for (const auto& [did, visibilityMap] : mContentLabelPrefs)
    {
        for (const auto& [label, visibility] : visibilityMap)
        {
            auto contentLabelPref = std::make_shared<AppBskyActor::ContentLabelPref>();

            if (!did.isEmpty())
                contentLabelPref->mLabelerDid = did;

            contentLabelPref->mLabel = label;
            contentLabelPref->mVisibility = visibility;
            preferences.push_back(std::move(contentLabelPref));
        }
    }

    auto savedFeedsPref = std::make_shared<AppBskyActor::SavedFeedsPref>(mSavedFeedsPref);
    preferences.push_back(std::move(savedFeedsPref));

    auto savedFeedsPrefV2 = std::make_shared<AppBskyActor::SavedFeedsPrefV2>(mSavedFeedsPrefV2);
    preferences.push_back(std::move(savedFeedsPrefV2));

    // With OAuth we do not get personal details and are not allowed to write them.
    // With a password we get them and must write them back or they get lost!
    if (!mPersonalDetailsPref.mJson.empty())
    {
        auto personalDetails = std::make_shared<AppBskyActor::PersonalDetailsPref>(mPersonalDetailsPref);
        preferences.push_back(std::move(personalDetails));
    }

    for (const auto& [_, feed] : mFeedViewPrefs)
    {
        auto feedViewPref = std::make_shared<AppBskyActor::FeedViewPref>(feed);
        preferences.push_back(std::move(feedViewPref));
    }

    auto threadView = std::make_shared<AppBskyActor::ThreadViewPref>(mThreadViewPref);
    preferences.push_back(std::move(threadView));

    auto mutedWords = std::make_shared<AppBskyActor::MutedWordsPref>(mMutedWordsPref);
    preferences.push_back(std::move(mutedWords));

    auto labelers = std::make_shared<AppBskyActor::LabelersPref>(mLabelersPref);
    preferences.push_back(std::move(labelers));

    auto postInteractionSettings = std::make_shared<AppBskyActor::PostInteractionSettingsPref>(mPostInteractionSettingsPref);
    preferences.push_back(std::move(postInteractionSettings));

    auto verification = std::make_shared<AppBskyActor::VerificationPrefs>(mVerificationPrefs);
    preferences.push_back(std::move(verification));

    for (const auto& unknown : mUnknownPrefs)
    {
        auto unknownPref = std::make_shared<AppBskyActor::UnknownPref>(unknown);
        preferences.push_back(std::move(unknownPref));
    }

    return preferences;
}

void UserPreferences::removeContentLabelPrefs(const QString& did)
{
    mContentLabelPrefs.erase(did);
}

UserPreferences::LabelVisibility UserPreferences::getLabelVisibility(const QString& did, const QString& label) const
{
    auto itDid = mContentLabelPrefs.find(did);

    if (itDid == mContentLabelPrefs.end())
        return LabelVisibility::UNKNOWN;

    const auto& labelVisibilityMap = itDid->second;
    auto itLabel = labelVisibilityMap.find(label);
    return itLabel != labelVisibilityMap.end() ? itLabel->second : LabelVisibility::UNKNOWN;
}

void UserPreferences::setLabelVisibility(const QString& did, const QString& label, LabelVisibility visibility)
{
    Q_ASSERT(visibility != LabelVisibility::UNKNOWN);

    if (visibility == LabelVisibility::UNKNOWN)
    {
        qWarning() << "Unknown visibility:" << label;
        return;
    }

    mContentLabelPrefs[did][label] = visibility;
}

void UserPreferences::removeLabelVisibility(const QString& did, const QString& label)
{
    qDebug() << "Remove label:" << label << "did:" << did;
    mContentLabelPrefs[did].erase(label);

    if (mContentLabelPrefs[did].empty())
    {
        qDebug() << "Remove did:" << did;
        mContentLabelPrefs.erase(did);
    }
}

const UserPreferences::FeedViewPref& UserPreferences::getFeedViewPref(const QString& feed) const
{
    static std::unordered_map<QString, FeedViewPref> DEFAULT_PREF;

    auto it = mFeedViewPrefs.find(feed);

    if (it != mFeedViewPrefs.end())
        return it->second;

    auto& feedViewPref = DEFAULT_PREF[feed];
    feedViewPref.mFeed = feed;
    return feedViewPref;
}

void UserPreferences::setFeedViewPref(const FeedViewPref& pref)
{
    Q_ASSERT(!pref.mFeed.isEmpty());

    if (pref.mFeed.isEmpty())
    {
        qWarning() << "Feed name missing";
        return;
    }

    mFeedViewPrefs[pref.mFeed] = pref;
}

}
