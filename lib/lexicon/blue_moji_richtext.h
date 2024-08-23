// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>

namespace ATProto::BlueMojiRichtext {

struct Formats_v0
{
    std::optional<QString> mPng128; // cid
    std::optional<QString> mWebp128; // cid
    std::optional<QString> mGif128; // cid
    bool mApng128 = false;
    bool mLottie = false;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Formats_v0>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "blue.moji.richtext.facet#formats_v0";
};

struct FacetBlueMoji
{
    QString mDid;
    QString mName;
    std::optional<QString> mAlt;
    bool mAdultOnly = false;
    // TODO: labels
    std::variant<Formats_v0::SharedPtr> mFormats; // required

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<FacetBlueMoji>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "blue.moji.richtext.facet";
};

}
