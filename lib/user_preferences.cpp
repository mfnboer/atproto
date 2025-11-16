// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "user_preferences.h"

namespace ATProto {

UserPreferences::UserPreferences(const AppBskyActor::Preference::List& preferences)
{
    setPrefs(preferences);
}

void UserPreferences::setPrefs(const AppBskyActor::Preference::List& preferences)
{
    for (const auto& pref : preferences)
    {
        switch (pref->mType)
        {
        case AppBskyActor::PreferenceType::ADULT_CONTENT:
        {
            const auto& adultContent = std::get<AppBskyActor::AdultContentPref::SharedPtr>(pref->mItem);
            mAdultContent = adultContent->mEnabled;
            break;
        }
        case AppBskyActor::PreferenceType::CONTENT_LABEL:
        {
            const auto& contentLabel = std::get<AppBskyActor::ContentLabelPref::SharedPtr>(pref->mItem);

            if (contentLabel->mVisibility != LabelVisibility::UNKNOWN)
            {
                const QString did = contentLabel->mLabelerDid.value_or("");
                mContentLabelPrefs[did][contentLabel->mLabel] = contentLabel->mVisibility;
            }

            break;
        }
        case AppBskyActor::PreferenceType::SAVED_FEEDS:
        {
            const auto& savedFeed = std::get<AppBskyActor::SavedFeedsPref::SharedPtr>(pref->mItem);
            mSavedFeedsPref = *savedFeed;
            break;
        }
        case ATProto::AppBskyActor::PreferenceType::SAVED_FEEDS_V2:
        {
            const auto& savedFeedsV2 = std::get<AppBskyActor::SavedFeedsPrefV2::SharedPtr>(pref->mItem);
            mSavedFeedsPrefV2 = *savedFeedsV2;
            break;
        }
        case AppBskyActor::PreferenceType::PERSONAL_DETAILS:
        {
            const auto& personal = std::get<AppBskyActor::PersonalDetailsPref::SharedPtr>(pref->mItem);
            mPersonalDetailsPref = *personal;
            mBirthDate = personal->mBirthDate;
            break;
        }
        case AppBskyActor::PreferenceType::FEED_VIEW:
        {
            const auto& feedView = std::get<AppBskyActor::FeedViewPref::SharedPtr>(pref->mItem);
            mFeedViewPrefs[feedView->mFeed] = *feedView;
            break;
        }
        case AppBskyActor::PreferenceType::THREAD_VIEW:
        {
            const auto& threadView = std::get<AppBskyActor::ThreadViewPref::SharedPtr>(pref->mItem);
            mThreadViewPref = *threadView;
            break;
        }
        case AppBskyActor::PreferenceType::MUTED_WORDS:
        {
            const auto& mutedWords = std::get<AppBskyActor::MutedWordsPref::SharedPtr>(pref->mItem);
            mMutedWordsPref = *mutedWords;
            break;
        }
        case AppBskyActor::PreferenceType::LABELERS:
        {
            const auto& labelers = std::get<AppBskyActor::LabelersPref::SharedPtr>(pref->mItem);
            mLabelersPref = *labelers;
            break;
        }
        case AppBskyActor::PreferenceType::POST_INTERACTION_SETTINGS:
        {
            const auto& postInteractionSettings = std::get<AppBskyActor::PostInteractionSettingsPref::SharedPtr>(pref->mItem);
            mPostInteractionSettingsPref = *postInteractionSettings;
            break;
        }
        case AppBskyActor::PreferenceType::VERIFICATION:
        {
            const auto& verification = std::get<AppBskyActor::VerificationPrefs::SharedPtr>(pref->mItem);
            mVerificationPrefs = *verification;
            break;
        }
        case AppBskyActor::PreferenceType::UNKNOWN:
            const auto& unknowPref = std::get<AppBskyActor::UnknownPref::SharedPtr>(pref->mItem);
            mUnknownPrefs.push_back(*unknowPref);
            break;
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

AppBskyActor::Preference::List UserPreferences::toPreferenceList() const
{
    AppBskyActor::Preference::SharedPtr pref;
    AppBskyActor::Preference::List preferences;

    auto adultContentPref = std::make_shared<AppBskyActor::AdultContentPref>();
    adultContentPref->mEnabled = mAdultContent;
    pref = std::make_shared<AppBskyActor::Preference>();
    pref->mItem = std::move(adultContentPref);
    pref->mType = AppBskyActor::PreferenceType::ADULT_CONTENT;
    preferences.push_back(std::move(pref));

    for (const auto& [did, visibilityMap] : mContentLabelPrefs)
    {
        for (const auto& [label, visibility] : visibilityMap)
        {
            auto contentLabelPref = std::make_shared<AppBskyActor::ContentLabelPref>();

            if (!did.isEmpty())
                contentLabelPref->mLabelerDid = did;

            contentLabelPref->mLabel = label;
            contentLabelPref->mVisibility = visibility;
            pref = std::make_shared<AppBskyActor::Preference>();
            pref->mItem = std::move(contentLabelPref);
            pref->mType = AppBskyActor::PreferenceType::CONTENT_LABEL;
            preferences.push_back(std::move(pref));
        }
    }

    auto savedFeedsPref = std::make_shared<AppBskyActor::SavedFeedsPref>(mSavedFeedsPref);
    pref = std::make_shared<AppBskyActor::Preference>();
    pref->mItem = std::move(savedFeedsPref);
    pref->mType = AppBskyActor::PreferenceType::SAVED_FEEDS;
    preferences.push_back(std::move(pref));

    auto savedFeedsPrefV2 = std::make_shared<AppBskyActor::SavedFeedsPrefV2>(mSavedFeedsPrefV2);
    pref = std::make_shared<AppBskyActor::Preference>();
    pref->mItem = std::move(savedFeedsPrefV2);
    pref->mType = AppBskyActor::PreferenceType::SAVED_FEEDS_V2;
    preferences.push_back(std::move(pref));

    auto personalDetails = std::make_shared<AppBskyActor::PersonalDetailsPref>(mPersonalDetailsPref);
    pref = std::make_shared<AppBskyActor::Preference>();
    pref->mItem = std::move(personalDetails);
    pref->mType = AppBskyActor::PreferenceType::PERSONAL_DETAILS;
    preferences.push_back(std::move(pref));

    for (const auto& [_, feed] : mFeedViewPrefs)
    {
        auto feedViewPref = std::make_shared<AppBskyActor::FeedViewPref>(feed);
        pref = std::make_shared<AppBskyActor::Preference>();
        pref->mItem = std::move(feedViewPref);
        pref->mType = AppBskyActor::PreferenceType::FEED_VIEW;
        preferences.push_back(std::move(pref));
    }

    auto threadView = std::make_shared<AppBskyActor::ThreadViewPref>(mThreadViewPref);
    pref = std::make_shared<AppBskyActor::Preference>();
    pref->mItem = std::move(threadView);
    pref->mType = AppBskyActor::PreferenceType::THREAD_VIEW;
    preferences.push_back(std::move(pref));

    auto mutedWords = std::make_shared<AppBskyActor::MutedWordsPref>(mMutedWordsPref);
    pref = std::make_shared<AppBskyActor::Preference>();
    pref->mItem = std::move(mutedWords);
    pref->mType = AppBskyActor::PreferenceType::MUTED_WORDS;
    preferences.push_back(std::move(pref));

    auto labelers = std::make_shared<AppBskyActor::LabelersPref>(mLabelersPref);
    pref = std::make_shared<AppBskyActor::Preference>();
    pref->mItem = std::move(labelers);
    pref->mType = AppBskyActor::PreferenceType::LABELERS;
    preferences.push_back(std::move(pref));

    auto postInteractionSettings = std::make_shared<AppBskyActor::PostInteractionSettingsPref>(mPostInteractionSettingsPref);
    pref = std::make_shared<AppBskyActor::Preference>();
    pref->mItem = std::move(postInteractionSettings);
    pref->mType = AppBskyActor::PreferenceType::POST_INTERACTION_SETTINGS;
    preferences.push_back(std::move(pref));

    auto verification = std::make_shared<AppBskyActor::VerificationPrefs>(mVerificationPrefs);
    pref = std::make_shared<AppBskyActor::Preference>();
    pref->mItem = std::move(verification);
    pref->mType = AppBskyActor::PreferenceType::VERIFICATION;
    preferences.push_back(std::move(pref));

    for (const auto& unknown : mUnknownPrefs)
    {
        auto unknownPref = std::make_shared<AppBskyActor::UnknownPref>(unknown);
        pref = std::make_shared<AppBskyActor::Preference>();
        pref->mItem = std::move(unknownPref);
        pref->mType = AppBskyActor::PreferenceType::UNKNOWN;
        preferences.push_back(std::move(pref));
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
