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

}
