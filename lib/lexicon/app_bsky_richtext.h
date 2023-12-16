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

// app.bsky.richtext.facet#tag
struct FacetTag
{
    QString mTag;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<FacetTag>;
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
            TAG,
            UNKNOWN
        };

        static Type stringToType(const QString& str);

        std::variant<FacetMention::Ptr, FacetLink::Ptr, FacetTag::Ptr> mFeature;
        Type mType;
    };

    FacetByteSlice mIndex;
    std::vector<Feature> mFeatures;

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Facet>;
    static Ptr fromJson(const QJsonObject& json);
};
using FacetList = std::vector<Facet::Ptr>;

/**
 * @brief applyFacets Replace the links in the text by HTML href anchors
 * @param text
 * @param facets
 * @param linkColor If provided a style element to set the color will be added to the href anchor
 * @return HTML version of the text with links in it
 */
QString applyFacets(const QString& text, const FacetList& facets, const QString& linkColor = "");

}
