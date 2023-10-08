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

    explicit UserPreferences(const AppBskyActor::PreferenceList& preferences);

private:
    void setPrefs(const AppBskyActor::PreferenceList& preferences);

    std::optional<QDateTime> mBirthDate;

    bool mAdultContent;
    std::unordered_map<QString, LabelVisibility> mContentLabelPrefs;

    std::unordered_map<QString, FeedViewPref> mFeedViewPrefs;
};

}
