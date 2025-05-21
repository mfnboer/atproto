// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonObject>

namespace ATProto::AppBskyEmbed {

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
