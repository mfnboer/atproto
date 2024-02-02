// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "rich_text_master.h"
#include "at_regex.h"
#include "tlds.h"

namespace ATProto {

std::vector<RichTextMaster::HtmlCleanupReplacement> RichTextMaster::sHtmlCleanupReplacements;

void RichTextMaster::addHtmlClenupReplacement(const QRegularExpression& from, const QString& to)
{
    sHtmlCleanupReplacements.push_back(HtmlCleanupReplacement{from, to});
}

QString RichTextMaster::toCleanedHtml(const QString& text)
{   
    // Sometimes posts have an ObjectReplacementCharacter in it. They should not, seems
    // a bug in the Bluesky app. QML refuses to display such texts, so we replace
    // them by whitespace.
    auto html = text.toHtmlEscaped()
                    .replace('\n', "<br>")
                    .replace(QChar::ObjectReplacementCharacter, ' ');

    for (const auto& replacement : sHtmlCleanupReplacements)
        html = html.replace(replacement.mFrom, replacement.mTo);

    return html;
}

QString RichTextMaster::plainToHtml(const QString& text)
{
    const QString html = toCleanedHtml(text);

    // Preserve white space
    return QString("<span style=\"white-space: pre-wrap\">%1</span>").arg(html);
}

QString RichTextMaster::getFormattedPostText(const ATProto::AppBskyFeed::Record::Post& post, const QString& linkColor)
{
    if (post.mFacets.empty())
        return plainToHtml(post.mText);
    else
        return ATProto::AppBskyRichtext::applyFacets(post.mText, post.mFacets, linkColor);
}

QString RichTextMaster::getFormattedFeedDescription(const ATProto::AppBskyFeed::GeneratorView& feed, const QString& linkColor)
{
    if (!feed.mDescription)
        return {};

    if (feed.mDescriptionFacets.empty())
        return linkiFy(*feed.mDescription, linkColor);

    return ATProto::AppBskyRichtext::applyFacets(*feed.mDescription, feed.mDescriptionFacets,
                                                 linkColor);
}

QString RichTextMaster::getFormattedListDescription(const ATProto::AppBskyGraph::ListView& list, const QString& linkColor)
{
    if (!list.mDescription)
        return {};

    if (list.mDescriptionFacets.empty())
        return linkiFy(*list.mDescription, linkColor);

    return ATProto::AppBskyRichtext::applyFacets(*list.mDescription, list.mDescriptionFacets,
                                                 linkColor);
}

QString RichTextMaster::linkiFy(const QString& text, const QString& colorName)
{
    const auto facets = RichTextMaster::parseFacets(text);
    QString linkified = "<span style=\"white-space: pre-wrap\">";

    int pos = 0;

    for (const auto& facet : facets)
    {
        if (facet.mType == ParsedMatch::Type::MENTION ||
            facet.mType == ParsedMatch::Type::LINK)
        {
            const auto before = text.sliced(pos, facet.mStartIndex - pos);
            linkified.append(toCleanedHtml(before));
            const QString ref = facet.mType == ParsedMatch::Type::MENTION || facet.mMatch.startsWith("http") ?
                                    facet.mMatch : "https://" + facet.mMatch;
            QString link = QString("<a href=\"%1\" style=\"color: %3;\">%2</a>").arg(ref, facet.mMatch, colorName);
            linkified.append(link);
            pos = facet.mEndIndex;
        }
    }

    linkified.append(toCleanedHtml(text.sliced(pos)));
    linkified.append("</span>");
    return linkified;
}

RichTextMaster::RichTextMaster(Client& client) :
    Presence(),
    mClient(client)
{
}

void RichTextMaster::resolveFacets(const QString& text, std::vector<ParsedMatch> facets, int facetIndex,
                   const FacetsResolvedCb& cb)
{
    Q_ASSERT(cb);
    for (int i = facetIndex; i < (int)facets.size(); ++i)
    {
        auto& facet = facets[i];

        switch (facet.mType) {
        case ParsedMatch::Type::LINK:
            facet.mRef = facet.mMatch.startsWith("http") ? facet.mMatch : "https://" + facet.mMatch;
            facet.mMatch = shortenWebLink(facet.mMatch);
            break;
        case ParsedMatch::Type::MENTION:
            // The @-character is not part of the handle!
            mClient.resolveHandle(facet.mMatch.sliced(1),
                [this, presence=getPresence(), i, text, facets, cb](const QString& did){
                    if (!presence)
                        return;

                    auto newFacets = facets;
                    newFacets[i].mRef = did;
                    resolveFacets(text, newFacets, i + 1, cb);
                },
                [this, presence=getPresence(), i, text, facets, cb](const QString& error, const QString& msg){
                    if (!presence)
                        return;

                    qWarning() << "Could not resolve handle:" << error << " - " << msg << "match:" << facets[i].mMatch;
                    resolveFacets(text, facets, i + 1, cb);
                });
            return;
        case ParsedMatch::Type::TAG:
            // The reference is the text from the hashtag
            facet.mRef = facet.mMatch.sliced(1);
            break;
        case ParsedMatch::Type::PARTIAL_MENTION:
            break;
        case ParsedMatch::Type::UNKNOWN:
            Q_ASSERT(false);
            break;
        }
    }

    addFacets(text, facets, cb);
}

static int convertIndextoUtf8Index(int index, const QString& str)
{
    qDebug() << "UTF-8:" << str.sliced(0, index).toUtf8();
    return str.sliced(0, index).toUtf8().length();
}

void RichTextMaster::addFacets(const QString& text, const std::vector<ParsedMatch>& facets,
               const FacetsResolvedCb& cb)
{
    int pos = 0;
    AppBskyRichtext::FacetList resolvedFacets;
    QString shortenedText;

    for (const auto& f : facets)
    {
        if (f.mRef.isEmpty())
            continue;

        shortenedText += text.sliced(pos, f.mStartIndex - pos);
        const int start = convertIndextoUtf8Index(shortenedText.size(), shortenedText);
        shortenedText += f.mMatch;
        const int end = convertIndextoUtf8Index(shortenedText.size(), shortenedText);
        pos = f.mEndIndex;

        auto facet = std::make_unique<AppBskyRichtext::Facet>();
        facet->mIndex.mByteStart = start;
        facet->mIndex.mByteEnd = end;

        AppBskyRichtext::Facet::Feature feature;
        feature.mType = f.mType;

        switch (feature.mType)
        {
        case AppBskyRichtext::Facet::Feature::Type::LINK:
        {
            auto link = std::make_unique<AppBskyRichtext::FacetLink>();
            link->mUri = f.mRef;
            feature.mFeature = std::move(link);
            break;
        }
        case AppBskyRichtext::Facet::Feature::Type::MENTION:
        {
            auto mention = std::make_unique<AppBskyRichtext::FacetMention>();
            mention->mDid = f.mRef;
            feature.mFeature = std::move(mention);
            break;
        }
        case AppBskyRichtext::Facet::Feature::Type::TAG:
        {
            auto tag = std::make_unique<AppBskyRichtext::FacetTag>();
            tag->mTag = f.mRef;
            feature.mFeature = std::move(tag);
            break;
        }
        case AppBskyRichtext::Facet::Feature::Type::PARTIAL_MENTION:
            continue;
        case AppBskyRichtext::Facet::Feature::Type::UNKNOWN:
            Q_ASSERT(false);
            qWarning() << "Unknown facet type";
            continue;
        }

        facet->mFeatures.push_back(std::move(feature));
        resolvedFacets.push_back(std::move(facet));
    }

    if (pos < text.size())
        shortenedText += text.sliced(pos);

    cb(shortenedText, std::move(resolvedFacets));
}

QString RichTextMaster::shortenWebLink(const QString& link)
{
    static const QRegularExpression httpRE(R"(https?:\/\/([^\/]+)\/(.{0,12})(.*))");
    static const QRegularExpression httpMainOnlyRE(R"(https?:\/\/(.+))");
    static const QRegularExpression wwwRE(R"(([a-zA-Z0-9][-a-zA-Z0-9]*\.[^\/]+)\/(.{0,12})(.*))");
    static const QRegularExpression wwwMainRE(R"(([a-zA-Z0-9][-a-zA-Z0-9]*\..+))");

    QRegularExpressionMatch match;
    for (const auto& re : { httpRE, httpMainOnlyRE, wwwRE, wwwMainRE})
    {
        match = re.match(link);

        if (match.hasMatch())
            break;
    }

    if (!match.hasMatch())
    {
        qWarning() << "Cannot shorten link:" << link;
        return link;
    }

    const QString& host = match.captured(1);
    const QString& remaining = match.captured(2);
    const QString& elide = match.captured(3);

    if (remaining.isEmpty())
        return host;

    if (elide.size() < 4)
        return QString("%1/%2%3").arg(host, remaining, elide);

    return QString("%1/%2...").arg(host, remaining);
}

static std::vector<RichTextMaster::ParsedMatch> parseMatches(RichTextMaster::ParsedMatch::Type type, const QString& text, const QRegularExpression& re, int group)
{
    std::vector<RichTextMaster::ParsedMatch> matches;

    // HACK: prefixing a space to match a mention at the start of the text
    for (const auto& match : re.globalMatch(' ' + text))
    {
        RichTextMaster::ParsedMatch parsedMatch;
        parsedMatch.mMatch = match.captured(group);
        parsedMatch.mStartIndex = match.capturedStart(group) - 1; // -1 for the hack
        parsedMatch.mEndIndex = match.capturedEnd(group) - 1;
        parsedMatch.mType = type;
        matches.push_back(parsedMatch);
    }

    return matches;
}

std::vector<RichTextMaster::ParsedMatch> RichTextMaster::parseTags(const QString& text)
{
    static const QRegularExpression reTag(R"([$|\W](#\w+))");
    const auto tags = parseMatches(ParsedMatch::Type::TAG, text, reTag, 1);

    for (const auto& tag : tags)
        qDebug() << "Tag:" << tag.mMatch << "start:" << tag.mStartIndex << "end:" << tag.mEndIndex;

    return tags;
}

std::vector<RichTextMaster::ParsedMatch> RichTextMaster::parsePartialMentions(const QString& text)
{
    static const QRegularExpression rePartialMention(R"([$|\W](@[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?))");
    const auto partialMentions = parseMatches(ParsedMatch::Type::PARTIAL_MENTION, text, rePartialMention, 1);

    for (const auto& partialMention : partialMentions)
        qDebug() << "Partial mention:" << partialMention.mMatch << "start:" << partialMention.mStartIndex << "end:" << partialMention.mEndIndex;

    return partialMentions;
}

std::vector<RichTextMaster::ParsedMatch> RichTextMaster::parseMentions(const QString& text)
{
    static const QRegularExpression reMention(QString(R"([$|\W](@%1))").arg(ATRegex::HANDLE.pattern()));
    const auto mentions = parseMatches(ParsedMatch::Type::MENTION, text, reMention, 1);

    for (const auto& mention : mentions)
        qDebug() << "Mention:" << mention.mMatch << "start:" << mention.mStartIndex << "end:" << mention.mEndIndex;

    return mentions;
}

std::vector<RichTextMaster::ParsedMatch> RichTextMaster::parseLinks(const QString& text)
{
    static const QRegularExpression reLink(R"([$|\W]((https?:\/\/)?[a-zA-Z0-9][-a-zA-Z0-9\.]{0,256}\.[a-zA-Z0-9()]{1,6}([-a-zA-Z0-9()@:%_\+\.,~#\?&/=]*[-a-zA-Z0-9()@%_\+~#/=])?))");
    auto links = parseMatches(ParsedMatch::Type::LINK, text, reLink, 1);

    for (int i = 0; i < (int)links.size();)
    {
        const auto& link = links[i];
        QUrl url(link.mMatch);

        if (link.mStartIndex > 0 && text[link.mStartIndex - 1] == '@')
        {
            // If there is an @-symbol just before what seems to be a link, it is not a link.
            qDebug() << "Not a link, looks like a mention:" << link.mMatch;
            links.erase(links.begin() + i);
        }
        else if (!url.isValid())
        {
            qDebug() << "Invalid URL:" << link.mMatch;
            links.erase(links.begin() + i);
        }
        else if (!link.mMatch.startsWith("http") && !isValidTLD(link.mMatch.split('/')[0].section('.', -1)))
        {
            qDebug() << "Invalid TLD:" << link.mMatch;
            links.erase(links.begin() + i);
        }
        else
        {

            qDebug() << "Link:" << link.mMatch << "start:" << link.mStartIndex << "end:" << link.mEndIndex;
            ++i;
        }
    }

    return links;
}

static void addToSortedMatches(std::map<int, RichTextMaster::ParsedMatch>& sortedMatches,
                               const std::vector<RichTextMaster::ParsedMatch>& matches)
{
    for (const auto& match : matches)
    {
        if (sortedMatches.count(match.mStartIndex))
        {
            Q_ASSERT(match.mType == RichTextMaster::ParsedMatch::Type::MENTION &&
                     sortedMatches[match.mStartIndex].mType == RichTextMaster::ParsedMatch::Type::PARTIAL_MENTION);
            qDebug() << "Two matches with start index:" << match.mStartIndex
                     << match.mMatch << sortedMatches[match.mStartIndex].mMatch;
        }

        sortedMatches[match.mStartIndex] = match;
    }
}

std::vector<RichTextMaster::ParsedMatch> RichTextMaster::parseFacets(const QString& text)
{
    const auto tags = parseTags(text);
    const auto partialMentions = parsePartialMentions(text);
    const auto mentions = parseMentions(text);
    const auto links = parseLinks(text);
    std::vector<RichTextMaster::ParsedMatch> facets;
    facets.reserve(mentions.size() + links.size());

    std::map<int, ParsedMatch> sortedMatches; // sorted on start index
    addToSortedMatches(sortedMatches, tags);
    addToSortedMatches(sortedMatches, partialMentions);
    addToSortedMatches(sortedMatches, mentions);
    addToSortedMatches(sortedMatches, links);

    int pos = 0;

    for (const auto& [_, match] : sortedMatches)
    {
        // This happens if a detected link overlaps with a detected mention.
        if (match.mStartIndex < pos)
        {
            // A partial mention may not be a partial mention but part of a link
            Q_ASSERT(match.mType == RichTextMaster::ParsedMatch::Type::PARTIAL_MENTION);
            qWarning() << "Overlapping facets at index:" << match.mStartIndex << match.mMatch;
            continue;
        }

        facets.push_back(match);
        pos = match.mEndIndex;
    }

    return facets;
}

std::vector<QString> RichTextMaster::getFacetTags(AppBskyFeed::Record::Post& post)
{
    std::vector<QString> tags;

    for (const auto& facet : post.mFacets)
    {
        for (const auto& feature : facet->mFeatures)
        {
            if (feature.mType == ATProto::AppBskyRichtext::Facet::Feature::Type::TAG)
            {
                const auto& facetTag = std::get<ATProto::AppBskyRichtext::FacetTag::Ptr>(feature.mFeature);
                tags.push_back(facetTag->mTag);
            }
        }
    }

    return tags;
}

}
