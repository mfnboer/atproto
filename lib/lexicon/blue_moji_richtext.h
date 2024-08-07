// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>

namespace ATProto::BlueMojiRichtext {

struct FacetBlueMoji
{
    QString mName;
    QString mUri;
    std::optional<QString> mAlt;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<FacetBlueMoji>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "blue.moji.richtext.facet#bluemoji";
};

}
