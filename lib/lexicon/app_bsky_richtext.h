// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>

namespace ATProto::AppBskyRichtext {

// app.bsky.richtext.facet#byteSlice
struct FacetByteSlice
{
    int mByteStart; // 0 or more, inclusive
    int mByteEnd; // 0 or more, exclusive

    static FacetByteSlice fromJson(const QJsonObject& json);
};

// app.bsky.richtext.facet#mention
struct FacetMention
{
    QString mDid;

    using Ptr = std::unique_ptr<FacetMention>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.richtext.facet#link
struct FacetLink
{
    QString mUri;

    using Ptr = std::unique_ptr<FacetLink>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.richtext.facet
struct Facet
{
    struct Feature
    {
        enum class Type
        {
            MENTION,
            LINK,
            UNKNOWN
        };

        static Type stringToType(const QString& str);

        std::variant<FacetMention::Ptr, FacetLink::Ptr> mFeature;
        Type mType;
    };

    FacetByteSlice mIndex;
    std::vector<Feature> mFeatures;

    using Ptr = std::unique_ptr<Facet>;
    static Ptr fromJson(const QJsonObject& json);
};

}
