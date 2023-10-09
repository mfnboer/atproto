// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "lexicon/app_bsky_actor.h"
#include <unordered_map>

namespace ATProto {

class UserPreferences
{
public:
    using LabelVisibility = AppBskyActor::ContentLabelPref::Visibility;
    using FeedViewPref = AppBskyActor::FeedViewPref;
    using ContentLabelPrefs = std::unordered_map<QString, LabelVisibility>;

    UserPreferences() = default;
    explicit UserPreferences(const AppBskyActor::PreferenceList& preferences);

    std::optional<QDateTime> getBirthDate() const { return mBirthDate; }
    bool getAdultContent() const { return mAdultContent; }
    const ContentLabelPrefs& getContentLabelPrefs() const { return mContentLabelPrefs; }
    LabelVisibility getLabelVisibility(const QString& label) const;
    const FeedViewPref* getFeedViewPref(const QString& feed) const;

private:
    void setPrefs(const AppBskyActor::PreferenceList& preferences);

    std::optional<QDateTime> mBirthDate;

    bool mAdultContent = false;
    ContentLabelPrefs mContentLabelPrefs;

    std::unordered_map<QString, FeedViewPref> mFeedViewPrefs;
};

}