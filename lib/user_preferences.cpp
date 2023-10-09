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
                mContentLabelPrefs[contentLabel->mLabel] = contentLabel->mVisibility;

            break;
        }
        case AppBskyActor::PreferenceType::SAVED_FEEDS:
            break;
        case AppBskyActor::PreferenceType::PERSONAL_DETAILS:
        {
            const auto& personal = std::get<AppBskyActor::PersonalDetailsPref::Ptr>(pref->mItem);
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
            break;
        case AppBskyActor::PreferenceType::UNKNOWN:
            break;
        }
    }
}

UserPreferences::LabelVisibility UserPreferences::getLabelVisibility(const QString& label) const
{
    auto it = mContentLabelPrefs.find(label);
    return it != mContentLabelPrefs.end() ? it->second : LabelVisibility::UNKNOWN;
}

const UserPreferences::FeedViewPref& UserPreferences::getFeedViewPref(const QString& feed) const
{
    static const FeedViewPref DEFAULT_PREF;

    auto it = mFeedViewPrefs.find(feed);
    return it != mFeedViewPrefs.end() ? it->second : DEFAULT_PREF;
}

}
