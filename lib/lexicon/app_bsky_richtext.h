// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>
#include <QString>
#include <set>

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

    using SharedPtr = std::shared_ptr<FacetMention>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.richtext.facet#link
struct FacetLink
{
    QString mUri;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<FacetLink>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// app.bsky.richtext.facet#tag
struct FacetTag
{
    QString mTag;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<FacetTag>;
    static SharedPtr fromJson(const QJsonObject& json);
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

        std::variant<FacetMention::SharedPtr, FacetLink::SharedPtr, FacetTag::SharedPtr> mFeature;
        Type mType;
    };

    FacetByteSlice mIndex;
    std::vector<Feature> mFeatures;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Facet>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

/**
 * @brief applyFacets Replace the links in the text by HTML href anchors
 * @param text
 * @param facets
 * @param linkColor If provided a style element to set the color will be added to the href anchor
 * @return HTML version of the text with links in it
 */
QString applyFacets(const QString& text, const Facet::List& facets, const QString& linkColor = "", const std::set<QString>& emphasizeHashtags = {});

}
