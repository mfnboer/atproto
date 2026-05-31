// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "lexicon.h"
#include "app_bsky_actor.h"
#include "com_atproto_label.h"
#include "com_atproto_repo.h"
#include <QColor>
#include <QJsonObject>

namespace ATProto::AppBskyEmbed {

// app.bsky.embed.external#external
struct ExternalExternal
{
    QString mUri;
    QString mTitle;
    QString mDescription;
    Blob::SharedPtr mThumb; // optional: max 1,000,000 bytes mime: image/*
    static constexpr int MAX_BYTES_THUMB = 1'000'000;

    // The URIs of the Atmosphere records representing this external content, if it exists.
    // Example: a site.standard.document record.
    ComATProtoRepo::StrongRef::List mAssociatedRefs;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ExternalExternal>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.embed.external
struct External
{
    ExternalExternal::SharedPtr mExternal;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<External>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.embed.external";
};

struct ColorRGB
{
    int mR;
    int mG;
    int mB;

    QColor toColor() const;
    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ColorRGB>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.embed.external#ColorRGB";
};

struct ViewExternalSourceTheme
{
    ColorRGB::SharedPtr mBackgroundRGB; // optional
    ColorRGB::SharedPtr mForegroundRGB; // optional
    ColorRGB::SharedPtr mAccentRGB; // optional
    ColorRGB::SharedPtr mAccentForegroundRGB; // optional

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ViewExternalSourceTheme>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.embed.external#viewExternalSourceTheme";
};

struct ViewExternalSource
{
    QString mUri;
    std::optional<QString> mIcon;
    QString mTitle;
    std::optional<QString> mDescription;
    ViewExternalSourceTheme::SharedPtr mTheme; // optional

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ViewExternalSource>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.embed.external#viewExternalSource";
};

// app.bsky.embed.external#viewExternal
struct ExternalViewExternal
{
    QString mUri;
    QString mTitle;
    QString mDescription;
    std::optional<QString> mThumb;
    std::optional<QDateTime> mCreatedAt;
    std::optional<QDateTime> mUpdatedAt;
    std::optional<int> mReadingTime; // in minutes
    ComATProtoLabel::Label::List mLabels;
    ViewExternalSource::SharedPtr mSource; // optional
    ComATProtoRepo::StrongRef::List mAssociatedRefs;
    AppBskyActor::ProfileViewBasic::List mAssociatedProfiles;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ExternalViewExternal>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.embed.external#view
struct ExternalView
{
    ExternalViewExternal::SharedPtr mExternal; // required

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ExternalView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.embed.external#view";
};

}
