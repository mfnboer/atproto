// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "lexicon.h"
#include <QJsonObject>

namespace ATProto::AppBskyEmbed {

// app.bsky.embed.external#external
struct ExternalExternal
{
    QString mUri;
    QString mTitle;
    QString mDescription;
    Blob::SharedPtr mThumb; // optional: max 1,000,000 bytes mime: image/*

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

// app.bsky.embed.external#viewExternal
struct ExternalViewExternal
{
    QString mUri;
    QString mTitle;
    QString mDescription;
    std::optional<QString> mThumb;

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
