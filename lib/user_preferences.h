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
    using DidLabelPair = std::pair<QString, QString>; // empty did means global label

    struct DidLabelPairHash {
        size_t operator()(DidLabelPair dlp) const noexcept { return qHash(dlp.second); }
    };

    // TODO: map in map may be better: first on DID then on label.
    using ContentLabelPrefs = std::unordered_map<DidLabelPair, LabelVisibility, DidLabelPairHash>;
    using SavedFeedsPref = AppBskyActor::SavedFeedsPref;
    using MutedWordsPref = AppBskyActor::MutedWordsPref;
    using LabelersPref = AppBskyActor::LabelersPref;

    UserPreferences() = default;
    explicit UserPreferences(const AppBskyActor::PreferenceList& preferences);

    std::optional<QDateTime> getBirthDate() const { return mBirthDate; }

    bool getAdultContent() const { return mAdultContent; }
    void setAdultContent(bool enabled) { mAdultContent = enabled; }

    void removeContentLabelPrefs(const QString& did);

    LabelVisibility getLabelVisibility(const QString& did, const QString& label) const;
    void setLabelVisibility(const QString& did, const QString& label, LabelVisibility visibility);

    const FeedViewPref& getFeedViewPref(const QString& feed) const;
    void setFeedViewPref(const FeedViewPref& pref);

    const SavedFeedsPref& getSavedFeedsPref() const { return mSavedFeedsPref; }
    void setSavedFeedsPref(const SavedFeedsPref& pref) { mSavedFeedsPref = pref; }

    const MutedWordsPref& getMutedWordsPref() const { return mMutedWordsPref; }
    MutedWordsPref& getMutedWordsPref() { return mMutedWordsPref; }

    const LabelersPref& getLabelersPref() const { return mLabelersPref; }
    void setLabelersPref(const LabelersPref& pref) { mLabelersPref = pref; }

    AppBskyActor::PreferenceList toPreferenceList() const;

private:
    void setPrefs(const AppBskyActor::PreferenceList& preferences);

    std::optional<QDateTime> mBirthDate;

    bool mAdultContent = false;
    ContentLabelPrefs mContentLabelPrefs;

    AppBskyActor::SavedFeedsPref mSavedFeedsPref;
    AppBskyActor::PersonalDetailsPref mPersonalDetailsPref;
    std::unordered_map<QString, FeedViewPref> mFeedViewPrefs;
    AppBskyActor::ThreadViewPref mThreadViewPref;
    AppBskyActor::MutedWordsPref mMutedWordsPref;
    AppBskyActor::LabelersPref mLabelersPref;
    std::vector<AppBskyActor::UnknownPref> mUnknownPrefs;
};

}
