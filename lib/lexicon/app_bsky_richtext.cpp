// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "app_bsky_richtext.h"
#include "../rich_text_master.h"
#include "../xjson.h"

namespace ATProto::AppBskyRichtext {

QJsonObject FacetByteSlice::toJson() const
{
    QJsonObject json;
    json.insert("byteStart", mByteStart);
    json.insert("byteEnd", mByteEnd);
    return json;
}

FacetByteSlice FacetByteSlice::fromJson(const QJsonObject& json)
{
    FacetByteSlice byteSlice;
    XJsonObject root(json);
    byteSlice.mByteStart = root.getRequiredInt("byteStart");
    byteSlice.mByteEnd = root.getRequiredInt("byteEnd");
    return byteSlice;
}

QJsonObject FacetMention::toJson() const
{
    QJsonObject json;
    json.insert("$type", "app.bsky.richtext.facet#mention");
    json.insert("did", mDid);
    return json;
}

FacetMention::SharedPtr FacetMention::fromJson(const QJsonObject& json)
{
    auto mention = std::make_shared<FacetMention>();
    const XJsonObject root(json);
    mention->mDid = root.getRequiredString("did");
    return mention;
}

QJsonObject FacetLink::toJson() const
{
    QJsonObject json;
    json.insert("$type", "app.bsky.richtext.facet#link");
    json.insert("uri", mUri);
    return json;
}

FacetLink::SharedPtr FacetLink::fromJson(const QJsonObject& json)
{
    auto link = std::make_shared<FacetLink>();
    const XJsonObject root(json);
    link->mUri = root.getRequiredString("uri");
    return link;
}

QJsonObject FacetTag::toJson() const
{
    QJsonObject json;
    json.insert("$type", "app.bsky.richtext.facet#tag");
    json.insert("tag", mTag);
    return json;
}

FacetTag::SharedPtr FacetTag::fromJson(const QJsonObject& json)
{
    auto tag = std::make_shared<FacetTag>();
    const XJsonObject root(json);
    tag->mTag = root.getRequiredString("tag");
    return tag;
}

Facet::Feature::Type Facet::Feature::stringToType(const QString& str)
{
    if (str == "app.bsky.richtext.facet#link")
        return Type::LINK;
    if (str == "app.bsky.richtext.facet#mention")
        return Type::MENTION;
    if (str == "app.bsky.richtext.facet#tag")
        return Type::TAG;

    return Type::UNKNOWN;
}

QJsonObject Facet::toJson() const
{
    QJsonObject json;
    json.insert("index", mIndex.toJson());

    QJsonArray jsonArray;
    for (const auto& f : mFeatures)
    {
        QJsonObject featureJson;

        switch (f.mType)
        {
        case Feature::Type::LINK:
            featureJson = std::get<FacetLink::SharedPtr>(f.mFeature)->toJson();
            break;
        case Feature::Type::MENTION:
            featureJson = std::get<FacetMention::SharedPtr>(f.mFeature)->toJson();
            break;
        case Feature::Type::TAG:
            featureJson = std::get<FacetTag::SharedPtr>(f.mFeature)->toJson();
            break;
        case Feature::Type::PARTIAL_MENTION:
        case Feature::Type::UNKNOWN:
            Q_ASSERT(false);
            qWarning() << "Unkown facet type";
            continue;
        }

        jsonArray.append(featureJson);
    }

    json.insert("features", jsonArray);

    return json;
}

Facet::SharedPtr Facet::fromJson(const QJsonObject& json)
{
    auto facet = std::make_shared<Facet>();
    const XJsonObject root(json);
    facet->mIndex = FacetByteSlice::fromJson(root.getRequiredJsonObject("index"));
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
        case Feature::Type::TAG:
            feature.mFeature = FacetTag::fromJson(featureJson);
            break;
        case Feature::Type::PARTIAL_MENTION:
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

static QString createHtmlLink(const QString& linkText, const Facet::Feature& feature, const QString& linkColor, const std::set<QString>& emphasizeHashtags = {})
{
    const QString linkStyle = linkColor.isEmpty() ? "" : QString(" style=\"color: %1; text-decoration: none\"").arg(linkColor);

    switch (feature.mType)
    {
    case ATProto::AppBskyRichtext::Facet::Feature::Type::MENTION:
    {
        const auto& facetMention = std::get<ATProto::AppBskyRichtext::FacetMention::SharedPtr>(feature.mFeature);
        return QString("<a href=\"%1\"%3>%2</a>").arg(facetMention->mDid, linkText, linkStyle);
        break;
    }
    case ATProto::AppBskyRichtext::Facet::Feature::Type::LINK:
    {
        const auto& facetLink = std::get<ATProto::AppBskyRichtext::FacetLink::SharedPtr>(feature.mFeature);
        return QString("<a href=\"%1\"%3>%2</a>").arg(facetLink->mUri, linkText, linkStyle);
    }
    case ATProto::AppBskyRichtext::Facet::Feature::Type::TAG:
    {
        const auto& facetTag = std::get<ATProto::AppBskyRichtext::FacetTag::SharedPtr>(feature.mFeature);
        const QString normalizedTag = RichTextMaster::normalizeText(facetTag->mTag);

        if (emphasizeHashtags.contains(normalizedTag))
            return QString("<a href=\"#%1\"%3><b>%2</b></a>").arg(facetTag->mTag, linkText, linkStyle);

        return QString("<a href=\"#%1\"%3>%2</a>").arg(facetTag->mTag, linkText, linkStyle);
    }
    case ATProto::AppBskyRichtext::Facet::Feature::Type::PARTIAL_MENTION:
        break;
    case ATProto::AppBskyRichtext::Facet::Feature::Type::UNKNOWN:
        qWarning() << "Uknown facet type:" << int(feature.mType) << "link:" << linkText;
        break;
    }

    return {};
}

static std::map<int, HyperLink> buildStartLinkMap(const QByteArray& bytes,
        const AppBskyRichtext::FacetList& facets, const QString& linkColor,
        const std::set<QString>& emphasizeHashtags = {})
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
        link.mText = createHtmlLink(linkText, feature, linkColor, emphasizeHashtags);

        if (!link.mText.isEmpty())
            startLinkMap[link.mStart] = link;
    }

    return startLinkMap;
}

QString applyFacets(const QString& text, const FacetList& facets, const QString& linkColor, const std::set<QString>& emphasizeHashtags)
{
    const auto& bytes = text.toUtf8();
    const auto startLinkMap = buildStartLinkMap(bytes, facets, linkColor, emphasizeHashtags);

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
        result.append(RichTextMaster::toCleanedHtml(QString(before)));
        result.append(link.mText);
        bytePos = link.mEnd;
    }

    result.append(RichTextMaster::toCleanedHtml(QString(bytes.sliced(bytePos))));
    return QString("<span style=\"white-space: pre-wrap\">%1</span>").arg(result);
}

}
