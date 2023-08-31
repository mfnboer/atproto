// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_richtext.h"
#include "../xjson.h"

namespace ATProto::AppBskyRichtext {

FacetByteSlice FacetByteSlice::fromJson(const QJsonObject& json)
{
    FacetByteSlice byteSlice;
    XJsonObject root(json);
    byteSlice.mByteStart = root.getRequiredInt("byteStart");
    byteSlice.mByteEnd = root.getRequiredInt("byteEnd");
    return byteSlice;
}

FacetMention::Ptr FacetMention::fromJson(const QJsonObject& json)
{
    auto mention = std::make_unique<FacetMention>();
    const XJsonObject root(json);
    mention->mDid = root.getRequiredString("did");
    return mention;
}

FacetLink::Ptr FacetLink::fromJson(const QJsonObject& json)
{
    auto link = std::make_unique<FacetLink>();
    const XJsonObject root(json);
    link->mUri = root.getRequiredString("uri");
    return link;
}

Facet::Feature::Type Facet::Feature::stringToType(const QString& str)
{
    if (str == "app.bsky.richtext.facet#link")
        return Type::LINK;
    if (str == "app.bsky.richtext.facet#mention")
        return Type::MENTION;

    return Type::UNKNOWN;
}

Facet::Ptr Facet::fromJson(const QJsonObject& json)
{
    auto facet = std::make_unique<Facet>();
    const XJsonObject root(json);
    facet->mIndex = FacetByteSlice::fromJson(root.getRequiredObject("index"));
    const auto features = root.getRequiredArray("features");

    for (const auto& f: features)
    {
        if (!f.isObject())
            throw InvalidJsonException("Invalid facet feature");

        Feature feature;
        const QJsonObject featureJson = f.toObject();
        const XJsonObject featureRoot(featureJson);
        const QString type = featureRoot.getRequiredString("$type");
        feature.mType = Facet::Feature::stringToType(type);


        switch (feature.mType)
        {
        case Feature::Type::MENTION:
            feature.mFeature = FacetMention::fromJson(featureJson);
            break;
        case Feature::Type::LINK:
            feature.mFeature = FacetLink::fromJson(featureJson);
            break;
        case Feature::Type::UNKNOWN:
            qWarning() << "Unsupported facet feature type:" << type;
            break;
        }

        facet->mFeatures.push_back(std::move(feature));
    }

    return facet;
}

struct HyperLink
{
    int mStart;
    int mEnd;
    QString mText;
};

static QString createHtmlLink(const QString& linkText, const Facet::Feature& feature)
{
    switch (feature.mType)
    {
    case ATProto::AppBskyRichtext::Facet::Feature::Type::MENTION:
    {
        const auto& facetMention = std::get<ATProto::AppBskyRichtext::FacetMention::Ptr>(feature.mFeature);
        return QString("<a href=\"%1\">%2</a>").arg(facetMention->mDid, linkText);
        break;
    }
    case ATProto::AppBskyRichtext::Facet::Feature::Type::LINK:
    {
        const auto& facetLink = std::get<ATProto::AppBskyRichtext::FacetLink::Ptr>(feature.mFeature);
        return QString("<a href=\"%1\">%2</a>").arg(facetLink->mUri, linkText);
        break;
    }
    case ATProto::AppBskyRichtext::Facet::Feature::Type::UNKNOWN:
        qWarning() << "Uknown facet type:" << int(feature.mType) << "link:" << linkText;
        break;
    }

    return {};
}

static std::map<int, HyperLink> buildStartLinkMap(const QByteArray& bytes, const std::vector<AppBskyRichtext::Facet::Ptr>& facets)
{
    std::map<int, HyperLink> startLinkMap;

    for (const auto& facet : facets)
    {
        if (facet->mFeatures.empty())
        {
            qWarning() << "Empty facet:" << QString(bytes);
            continue;
        }

        // What to do with multiple features?
        if (facet->mFeatures.size() > 1)
            qWarning() << "Multiple features, taking only the first";

        HyperLink link;
        link.mStart = facet->mIndex.mByteStart;
        link.mEnd = facet->mIndex.mByteEnd;
        const int sliceSize = link.mEnd - link.mStart;

        if (link.mStart < 0 || link.mEnd > bytes.size() || sliceSize < 0)
        {
            qWarning() << "Invalid index in facet:" << QString(bytes);
            continue;
        }

        const auto linkText = QString(bytes.sliced(link.mStart, sliceSize));
        const auto& feature = facet->mFeatures.front();
        link.mText = createHtmlLink(linkText, feature);

        if (!link.mText.isEmpty())
            startLinkMap[link.mStart] = link;
    }

    return startLinkMap;
}

QString applyFacets(const QString& text, const std::vector<AppBskyRichtext::Facet::Ptr>& facets)
{
    const auto& bytes = text.toUtf8();
    const auto startLinkMap = buildStartLinkMap(bytes, facets);

    QString result;
    int bytePos = 0;

    for (const auto& [start, link] : startLinkMap)
    {
        if (start < bytePos)
        {
            qWarning() << "Overlapping facets:" << text;
            result.clear();
            bytePos = 0;
            break;
        }

        const auto before = bytes.sliced(bytePos, start - bytePos);
        result.append(QString(before).toHtmlEscaped());
        result.append(link.mText);
        bytePos = link.mEnd;
    }

    result.append(QString(bytes.sliced(bytePos)).toHtmlEscaped());
    qDebug() << "Orig:   " << text;
    qDebug() << "Faceted:" << result;
    return result;
}

}
