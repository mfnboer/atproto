// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "client.h"
#include "presence.h"
#include <set>

namespace ATProto {

class RichTextMaster : public Presence
{
public:
    using FacetsResolvedCb = std::function<void(const QString& text, AppBskyRichtext::FacetList facets)>;
    using HtmlCleanupFun = std::function<QString(const QString&)>;

    struct ParsedMatch
    {
        using Type = ATProto::AppBskyRichtext::Facet::Feature::Type;

        int mStartIndex;
        int mEndIndex;
        QString mMatch;
        Type mType;
        QString mRef;
    };

    static void setHtmlCleanup(const HtmlCleanupFun& cleanup);
    static QString toCleanedHtml(const QString& text);
    static QString plainToHtml(const QString& text);
    static QString getFormattedPostText(const ATProto::AppBskyFeed::Record::Post& post, const QString& linkColor, const std::set<QString>& emphasizeHashtags = {});
    static QString getFormattedFeedDescription(const ATProto::AppBskyFeed::GeneratorView& feed, const QString& linkColor);
    static QString getFormattedListDescription(const ATProto::AppBskyGraph::ListView& list, const QString& linkColor);
    static QString getFormattedStarterPackDescription(const ATProto::AppBskyGraph::StarterPack& starterPack, const QString& linkColor);
    static QString getFormattedLabelerDescription(const ATProto::AppBskyLabeler::LabelerView& labeler, const QString& linkColor);
    static QString getFormattedMessageText(const ATProto::ChatBskyConvo::MessageView& msg, const QString& linkColor);
    static std::vector<ParsedMatch> getEmbeddedLinks(const QString& text, const AppBskyRichtext::FacetList& facets);
    static QString linkiFy(const QString& text, const std::vector<ParsedMatch>& embeddedLinks, const QString& colorName);
    static QString normalizeText(const QString& text);

    explicit RichTextMaster(Client& client);

    void resolveFacets(const QString& text, std::vector<ParsedMatch> facets, int facetIndex,
                       bool shortenLinks, const FacetsResolvedCb& cb);
    void addFacets(const QString& text, const std::vector<ParsedMatch>& facets,
                   const FacetsResolvedCb& cb);

    static QString shortenWebLink(const QString& link);

    static std::vector<ParsedMatch> parsePartialMentions(const QString& text);
    static std::vector<ParsedMatch> parseMentions(const QString& text);
    static std::vector<ParsedMatch> parseLinks(const QString& text);
    static std::vector<ParsedMatch> parseTags(const QString& text);
    static bool isHashtag(const QString& text);

    // If two facets overlap, then the one with the lowest start index is taken
    static std::vector<ParsedMatch> parseFacets(const QString& text);

    static void insertEmbeddedLinksToFacets(
            const std::vector<ParsedMatch>& embeddedLinks,
            std::vector<ParsedMatch>& facets);
    static void removeFacetsOverlappingWithEmbeddedLinks(
            const std::vector<ParsedMatch>& embeddedLinks,
            std::vector<ParsedMatch>& facets);
    static std::vector<QString> getFacetTags(const AppBskyFeed::Record::Post& post);
    static std::vector<QString> getFacetLinks(const AppBskyFeed::Record::Post& post);

private:
    Client& mClient;

    static HtmlCleanupFun sHtmlCleanup;
};

}
