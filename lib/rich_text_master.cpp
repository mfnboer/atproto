// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "rich_text_master.h"
#include "at_regex.h"
#include "tlds.h"
#include <QUrl>
#include <ranges>

namespace ATProto {

static constexpr char const* RE_HASHTAG = R"(#[^[:punct:][:space:]]+)";

RichTextMaster::HtmlCleanupFun RichTextMaster::sHtmlCleanup;

void RichTextMaster::setHtmlCleanup(const HtmlCleanupFun& cleanup)
{
    sHtmlCleanup = cleanup;
}

QString RichTextMaster::toCleanedHtml(const QString& text)
{   
    // Sometimes posts have an ObjectReplacementCharacter in it. They should not, seems
    // a bug in the Bluesky app. QML refuses to display such texts, so we replace
    // them by whitespace.
    auto html = text.toHtmlEscaped()
                    .replace('\n', "<br>")
                    .replace(QChar::ObjectReplacementCharacter, ' ');

    if (sHtmlCleanup)
        html = sHtmlCleanup(html);

    return html;
}

QString RichTextMaster::plainToHtml(const QString& text)
{
    const QString html = toCleanedHtml(text);

    // Preserve white space
    return QString("<span style=\"white-space: pre-wrap\">%1</span>").arg(html);
}

QString RichTextMaster::getFormattedPostText(const ATProto::AppBskyFeed::Record::Post& post, const QString& linkColor, const std::set<QString>& emphasizeHashtags)
{
    if (post.mText.isEmpty())
        return {};

    if (post.mFacets.empty())
        return plainToHtml(post.mText);
    else
        return ATProto::AppBskyRichtext::applyFacets(post.mText, post.mFacets, linkColor, emphasizeHashtags);
}

QString RichTextMaster::getFormattedFeedDescription(const ATProto::AppBskyFeed::GeneratorView& feed, const QString& linkColor)
{
    if (!feed.mDescription || feed.mDescription->isEmpty())
        return {};

    if (feed.mDescriptionFacets.empty())
        return linkiFy(*feed.mDescription, {}, linkColor);

    return ATProto::AppBskyRichtext::applyFacets(*feed.mDescription, feed.mDescriptionFacets,
                                                 linkColor);
}

QString RichTextMaster::getFormattedListDescription(const ATProto::AppBskyGraph::ListView& list, const QString& linkColor)
{
    if (!list.mDescription || list.mDescription->isEmpty())
        return {};

    if (list.mDescriptionFacets.empty())
        return linkiFy(*list.mDescription, {}, linkColor);

    return ATProto::AppBskyRichtext::applyFacets(*list.mDescription, list.mDescriptionFacets,
                                                 linkColor);
}

QString RichTextMaster::getFormattedStarterPackDescription(const ATProto::AppBskyGraph::StarterPack& starterPack, const QString& linkColor)
{
    if (!starterPack.mDescription || starterPack.mDescription->isEmpty())
        return {};

    if (starterPack.mDescriptionFacets.empty())
        return linkiFy(*starterPack.mDescription, {}, linkColor);

    return ATProto::AppBskyRichtext::applyFacets(*starterPack.mDescription, starterPack.mDescriptionFacets,
                                                 linkColor);
}

QString RichTextMaster::getFormattedLabelerDescription(const ATProto::AppBskyLabeler::LabelerView& labeler, const QString& linkColor)
{
    const auto& description = labeler.mCreator->mDescription;

    if (!description || description->isEmpty())
        return {};

    return linkiFy(*description, {}, linkColor);
}

QString RichTextMaster::getFormattedMessageText(const ATProto::ChatBskyConvo::MessageView& msg, const QString& linkColor)
{
    if (msg.mText.isEmpty())
        return {};

    if (msg.mFacets.empty())
        return plainToHtml(msg.mText);
    else
        return ATProto::AppBskyRichtext::applyFacets(msg.mText, msg.mFacets, linkColor);
}

static int convertUtf8IndexToIndex(int utf8Index, const QByteArray& utf8Bytes)
{
    const QString str(utf8Bytes.sliced(0, utf8Index));
    return str.length();
}

std::vector<RichTextMaster::ParsedMatch> RichTextMaster::getEmbeddedLinks(const QString& text, const AppBskyRichtext::FacetList& facets)
{
    const auto& bytes = text.toUtf8();
    std::vector<RichTextMaster::ParsedMatch> embeddedLinks;

    for (const auto& facet : facets)
    {
        if (facet->mFeatures.size() == 1 && facet->mFeatures.front().mType == ParsedMatch::Type::LINK)
        {
            const int start = facet->mIndex.mByteStart;
            const int end = facet->mIndex.mByteEnd;
            const int sliceSize = end - start;

            if (start < 0 || end > bytes.size() || sliceSize < 0)
            {
                qWarning() << "Invalid index in facet:" << QString(bytes);
                continue;
            }

            const auto linkText = QString(bytes.sliced(start, sliceSize));

            if (linkText.isEmpty())
                continue;

            const auto feature = std::get<AppBskyRichtext::FacetLink::SharedPtr>(facet->mFeatures.front().mFeature);
            QUrl url(feature->mUri);
            qDebug() << "Link:" << linkText << "uri:" << url;

            if (linkText.contains(url.host()))
                continue;

            ParsedMatch link;
            link.mMatch = linkText;
            link.mRef = feature->mUri;
            link.mType = ParsedMatch::Type::LINK;
            link.mStartIndex = convertUtf8IndexToIndex(start, bytes);
            link.mEndIndex = convertUtf8IndexToIndex(end, bytes);

            embeddedLinks.push_back(link);
        }
    }

    return embeddedLinks;
}

QString RichTextMaster::linkiFy(const QString& text, const std::vector<ParsedMatch>& embeddedLinks, const QString& colorName)
{
    auto facets = RichTextMaster::parseFacets(text);
    insertEmbeddedLinksToFacets(embeddedLinks, facets);
    QString linkified = "<span style=\"white-space: pre-wrap\">";

    int pos = 0;

    for (const auto& facet : facets)
    {
        if (facet.mType == ParsedMatch::Type::MENTION ||
            facet.mType == ParsedMatch::Type::LINK ||
            facet.mType == ParsedMatch::Type::TAG)
        {
            const auto before = text.sliced(pos, facet.mStartIndex - pos);
            linkified.append(toCleanedHtml(before));
            QString ref;

            if (facet.mType == ParsedMatch::Type::MENTION || facet.mType == ParsedMatch::Type::TAG)
            {
                ref = facet.mMatch;
            }
            else
            {
                if (!facet.mRef.isEmpty())
                    ref = facet.mRef;
                else
                    ref = facet.mMatch.startsWith("http") ? facet.mMatch : "https://" + facet.mMatch;
            }

            QString link = QString("<a href=\"%1\" style=\"color: %3; text-decoration: none\">%2</a>").arg(ref, facet.mMatch, colorName);
            linkified.append(link);
            pos = facet.mEndIndex;
        }
    }

    linkified.append(toCleanedHtml(text.sliced(pos)));
    linkified.append("</span>");
    return linkified;
}

QString RichTextMaster::normalizeText(const QString& text)
{
    QLocale locale;
    const QString NFKD = text.normalized(QString::NormalizationForm_KD);
    QString normalized;

    for (const auto ch : NFKD)
    {
        switch (ch.category())
        {
        case QChar::Mark_NonSpacing:
        case QChar::Mark_SpacingCombining:
        case QChar::Mark_Enclosing:
            continue;
        default:
            break;
        }

        normalized.append(ch);
    }

    return locale.toLower(normalized);
}

RichTextMaster::RichTextMaster(Client& client) :
    Presence(),
    mClient(client)
{
}

void RichTextMaster::resolveFacets(const QString& text, std::vector<ParsedMatch> facets, int facetIndex,
                                   bool shortenLinks, const FacetsResolvedCb& cb)
{
    Q_ASSERT(cb);
    for (int i = facetIndex; i < (int)facets.size(); ++i)
    {
        auto& facet = facets[i];

        switch (facet.mType) {
        case ParsedMatch::Type::LINK:
            if (facet.mRef.isEmpty())
            {
                facet.mRef = facet.mMatch.startsWith("http") ? facet.mMatch : "https://" + facet.mMatch;

                if (shortenLinks)
                    facet.mMatch = shortenWebLink(facet.mMatch);
            }

            break;
        case ParsedMatch::Type::MENTION:
            // The @-character is not part of the handle!
            mClient.resolveHandle(facet.mMatch.sliced(1),
                [this, presence=getPresence(), i, text, facets, shortenLinks, cb](const QString& did){
                    if (!presence)
                        return;

                    auto newFacets = facets;
                    newFacets[i].mRef = did;
                    resolveFacets(text, newFacets, i + 1, shortenLinks, cb);
                },
                [this, presence=getPresence(), i, text, facets, shortenLinks, cb](const QString& error, const QString& msg){
                    if (!presence)
                        return;

                    qWarning() << "Could not resolve handle:" << error << " - " << msg << "match:" << facets[i].mMatch;
                    resolveFacets(text, facets, i + 1, shortenLinks, cb);
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

        auto facet = std::make_shared<AppBskyRichtext::Facet>();
        facet->mIndex.mByteStart = start;
        facet->mIndex.mByteEnd = end;

        AppBskyRichtext::Facet::Feature feature;
        feature.mType = f.mType;

        switch (feature.mType)
        {
        case AppBskyRichtext::Facet::Feature::Type::LINK:
        {
            auto link = std::make_shared<AppBskyRichtext::FacetLink>();
            link->mUri = f.mRef;
            feature.mFeature = std::move(link);
            break;
        }
        case AppBskyRichtext::Facet::Feature::Type::MENTION:
        {
            auto mention = std::make_shared<AppBskyRichtext::FacetMention>();
            mention->mDid = f.mRef;
            feature.mFeature = std::move(mention);
            break;
        }
        case AppBskyRichtext::Facet::Feature::Type::TAG:
        {
            auto tag = std::make_shared<AppBskyRichtext::FacetTag>();
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
    static const QRegularExpression reTag(QString(R"([$|\W](%1))").arg(RE_HASHTAG));
    static const QRegularExpression reNumberTag("^#[0-9]+$");

    const auto potentialTags = parseMatches(ParsedMatch::Type::TAG, text, reTag, 1);
    std::vector<RichTextMaster::ParsedMatch> tags;
    tags.reserve(potentialTags.size());

    for (const auto& tag : potentialTags)
    {
        // Exclude keycap emoji #️⃣: U+23 U+FE0F U+20E3
        if (tag.mMatch.startsWith("#\uFE0F"))
            continue;

        if (reNumberTag.match(tag.mMatch).hasMatch())
            continue;

        qDebug() << "Tag:" << tag.mMatch << "start:" << tag.mStartIndex << "end:" << tag.mEndIndex;
        tags.push_back(tag);
    }

    return tags;
}

bool RichTextMaster::isHashtag(const QString& text)
{
    static const QRegularExpression reTag(QString("^%1$").arg(RE_HASHTAG));
    static const QRegularExpression reNumberTag("^#[0-9]+$");

    // Exclude keycap emoji #️⃣: U+23 U+FE0F U+20E3 and number tags
    return text.startsWith('#') &&
           !text.startsWith("#\uFE0F") &&
           !reNumberTag.match(text).hasMatch() &&
           reTag.match(text).hasMatch();
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
            qWarning() << "Overlapping facets at index:" << match.mStartIndex << match.mMatch << (int)match.mType;

            // A (partial) mention may not be a partial mention but part of a link
            // A tag may not be a tag but part of a link
            // A link may not be a link but part of a tag, e.g. #example.com
            continue;
        }

        facets.push_back(match);
        pos = match.mEndIndex;
    }

    return facets;
}

void RichTextMaster::insertEmbeddedLinksToFacets(
        const std::vector<RichTextMaster::ParsedMatch>& embeddedLinks,
        std::vector<RichTextMaster::ParsedMatch>& facets)
{
    if (embeddedLinks.empty())
        return;

    removeFacetsOverlappingWithEmbeddedLinks(embeddedLinks, facets);

    std::map<int, ParsedMatch> sortedMatches; // sorted on start index
    addToSortedMatches(sortedMatches, facets);
    addToSortedMatches(sortedMatches, embeddedLinks);

    facets.clear();

    for (const auto& f : sortedMatches | std::views::values)
        facets.push_back(f);
}

static bool facetOverlaps(const RichTextMaster::ParsedMatch& facet, const std::vector<RichTextMaster::ParsedMatch>& otherFacets)
{
    for (const auto& other : otherFacets)
    {
        if (facet.mStartIndex < other.mEndIndex && facet.mEndIndex > other.mStartIndex)
            return true;
    }

    return false;
}

void RichTextMaster::removeFacetsOverlappingWithEmbeddedLinks(
        const std::vector<RichTextMaster::ParsedMatch>& embeddedLinks,
        std::vector<RichTextMaster::ParsedMatch>& facets)
{
    for (auto it = facets.begin(); it != facets.end(); )
    {
        if (facetOverlaps(*it, embeddedLinks))
            it = facets.erase(it);
        else
            ++it;
    }
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
                const auto& facetTag = std::get<ATProto::AppBskyRichtext::FacetTag::SharedPtr>(feature.mFeature);
                tags.push_back(facetTag->mTag);
            }
        }
    }

    return tags;
}

}
