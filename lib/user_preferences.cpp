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
        switch (pref->mType)
        {
        case AppBskyActor::PreferenceType::ADULT_CONTENT:
        {
            const auto& adultContent = std::get<AppBskyActor::AdultContentPref::Ptr>(pref->mItem);
            mAdultContent = adultContent->mEnabled;
            break;
        }
        case AppBskyActor::PreferenceType::CONTENT_LABEL:
        {
            const auto& contentLabel = std::get<AppBskyActor::ContentLabelPref::Ptr>(pref->mItem);

            if (contentLabel->mVisibility != LabelVisibility::UNKNOWN)
            {
                const DidLabelPair dlp{ contentLabel->mLabelerDid.value_or(""), contentLabel->mLabel };
                mContentLabelPrefs[dlp] = contentLabel->mVisibility;
            }

            break;
        }
        case AppBskyActor::PreferenceType::SAVED_FEEDS:
        {
            const auto& savedFeed = std::get<AppBskyActor::SavedFeedsPref::Ptr>(pref->mItem);
            mSavedFeedsPref = *savedFeed;
            break;
        }
        case AppBskyActor::PreferenceType::PERSONAL_DETAILS:
        {
            const auto& personal = std::get<AppBskyActor::PersonalDetailsPref::Ptr>(pref->mItem);
            mPersonalDetailsPref = *personal;
            mBirthDate = personal->mBirthDate;
            break;
        }
        case AppBskyActor::PreferenceType::FEED_VIEW:
        {
            const auto& feedView = std::get<AppBskyActor::FeedViewPref::Ptr>(pref->mItem);
            mFeedViewPrefs[feedView->mFeed] = *feedView;
            break;
        }
        case AppBskyActor::PreferenceType::THREAD_VIEW:
        {
            const auto& threadView = std::get<AppBskyActor::ThreadViewPref::Ptr>(pref->mItem);
            mThreadViewPref = *threadView;
            break;
        }
        case AppBskyActor::PreferenceType::MUTED_WORDS:
        {
            const auto& mutedWords = std::get<AppBskyActor::MutedWordsPref::Ptr>(pref->mItem);
            mMutedWordsPref = *mutedWords;
            break;
        }
        case AppBskyActor::PreferenceType::LABELERS:
        {
            const auto& labelers = std::get<AppBskyActor::LabelersPref::Ptr>(pref->mItem);
            mLabelersPref = *labelers;
            break;
        }
        case AppBskyActor::PreferenceType::UNKNOWN:
            const auto& unknowPref = std::get<AppBskyActor::UnknownPref::Ptr>(pref->mItem);
            mUnknownPrefs.push_back(*unknowPref);
            break;
        }
    }
}

AppBskyActor::PreferenceList UserPreferences::toPreferenceList() const
{
    AppBskyActor::Preference::Ptr pref;
    AppBskyActor::PreferenceList preferences;

    auto adultContentPref = std::make_unique<AppBskyActor::AdultContentPref>();
    adultContentPref->mEnabled = mAdultContent;
    pref = std::make_unique<AppBskyActor::Preference>();
    pref->mItem = std::move(adultContentPref);
    pref->mType = AppBskyActor::PreferenceType::ADULT_CONTENT;
    preferences.push_back(std::move(pref));

    for (const auto& [label, visibility] : mContentLabelPrefs)
    {
        auto contentLabelPref = std::make_unique<AppBskyActor::ContentLabelPref>();

        if (!label.first.isEmpty())
            contentLabelPref->mLabelerDid = label.first;

        contentLabelPref->mLabel = label.second;
        contentLabelPref->mVisibility = visibility;
        pref = std::make_unique<AppBskyActor::Preference>();
        pref->mItem = std::move(contentLabelPref);
        pref->mType = AppBskyActor::PreferenceType::CONTENT_LABEL;
        preferences.push_back(std::move(pref));
    }

    auto savedFeedsPred = std::make_unique<AppBskyActor::SavedFeedsPref>(mSavedFeedsPref);
    pref = std::make_unique<AppBskyActor::Preference>();
    pref->mItem = std::move(savedFeedsPred);
    pref->mType = AppBskyActor::PreferenceType::SAVED_FEEDS;
    preferences.push_back(std::move(pref));

    auto personalDetails = std::make_unique<AppBskyActor::PersonalDetailsPref>(mPersonalDetailsPref);
    pref = std::make_unique<AppBskyActor::Preference>();
    pref->mItem = std::move(personalDetails);
    pref->mType = AppBskyActor::PreferenceType::PERSONAL_DETAILS;
    preferences.push_back(std::move(pref));

    for (const auto& [_, feed] : mFeedViewPrefs)
    {
        auto feedViewPref = std::make_unique<AppBskyActor::FeedViewPref>(feed);
        pref = std::make_unique<AppBskyActor::Preference>();
        pref->mItem = std::move(feedViewPref);
        pref->mType = AppBskyActor::PreferenceType::FEED_VIEW;
        preferences.push_back(std::move(pref));
    }

    auto threadView = std::make_unique<AppBskyActor::ThreadViewPref>(mThreadViewPref);
    pref = std::make_unique<AppBskyActor::Preference>();
    pref->mItem = std::move(threadView);
    pref->mType = AppBskyActor::PreferenceType::THREAD_VIEW;
    preferences.push_back(std::move(pref));

    auto mutedWords = std::make_unique<AppBskyActor::MutedWordsPref>(mMutedWordsPref);
    pref = std::make_unique<AppBskyActor::Preference>();
    pref->mItem = std::move(mutedWords);
    pref->mType = AppBskyActor::PreferenceType::MUTED_WORDS;
    preferences.push_back(std::move(pref));

    auto labelers = std::make_unique<AppBskyActor::LabelersPref>(mLabelersPref);
    pref = std::make_unique<AppBskyActor::Preference>();
    pref->mItem = std::move(labelers);
    pref->mType = AppBskyActor::PreferenceType::LABELERS;
    preferences.push_back(std::move(pref));

    for (const auto& unknown : mUnknownPrefs)
    {
        auto unknownPref = std::make_unique<AppBskyActor::UnknownPref>(unknown);
        pref = std::make_unique<AppBskyActor::Preference>();
        pref->mItem = std::move(unknownPref);
        pref->mType = AppBskyActor::PreferenceType::UNKNOWN;
        preferences.push_back(std::move(pref));
    }

    return preferences;
}

void UserPreferences::removeContentLabelPrefs(const QString& did)
{
    for (auto it = mContentLabelPrefs.begin(); it != mContentLabelPrefs.end(); )
    {
        const auto didLabelPair = it->first;

        if (didLabelPair.first == did)
        {
            qDebug() << "Remove, did:" << didLabelPair.first << "label:" << didLabelPair.second;
            it = mContentLabelPrefs.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

UserPreferences::LabelVisibility UserPreferences::getLabelVisibility(const QString& did, const QString& label) const
{
    auto it = mContentLabelPrefs.find({ did, label });
    return it != mContentLabelPrefs.end() ? it->second : LabelVisibility::UNKNOWN;
}

void UserPreferences::setLabelVisibility(const QString& did, const QString& label, LabelVisibility visibility)
{
    Q_ASSERT(visibility != LabelVisibility::UNKNOWN);

    if (visibility == LabelVisibility::UNKNOWN)
    {
        qWarning() << "Unknown visibility:" << label;
        return;
    }

    mContentLabelPrefs[{ did, label }] = visibility;
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
