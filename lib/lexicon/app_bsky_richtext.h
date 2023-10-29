// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>

namespace ATProto::AppBskyRichtext {

// TODO: tags

// app.bsky.richtext.facet#byteSlice
struct FacetByteSlice
{
    int mByteStart; // 0 or more, inclusive
    int mByteEnd; // 0 or more, exclusive

    QJsonObject toJson() const;

    static FacetByteSlice fromJson(const QJsonObject& json);
};

// app.bsky.richtext.facet#mention
struct FacetMention
{
    QString mDid;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<FacetMention>;
    static Ptr fromJson(const QJsonObject& json);
};

// app.bsky.richtext.facet#link
struct FacetLink
{
    QString mUri;

    QJsonObject toJson() const;

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
            PARTIAL_MENTION,
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

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Facet>;
    static Ptr fromJson(const QJsonObject& json);
};

/**
 * @brief applyFacets Replace the links in the text by HTML href anchors
 * @param text
 * @param facets
 * @return HTML version of the text with links in it
 */
QString applyFacets(const QString& text, const std::vector<AppBskyRichtext::Facet::Ptr>& facets);

}
