// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_master.h"
#include "at_regex.h"
#include "tlds.h"
#include <QTimer>

namespace ATProto {

QString PostMaster::plainToHtml(const QString& text)
{
    // Sometime post have an ObjectReplacementCharacter in it. They should not, seems
    // a bug in the Bluesky app. QML refuses to display such texts, so we replace
    // them by whitespace.
    const auto html = text.toHtmlEscaped()
                          .replace('\n', "<br>")
                          .replace(QChar::ObjectReplacementCharacter, ' ');

    // Preserve white space
    return QString("<span style=\"white-space: pre-wrap\">%1</span>").arg(html);
}

QString PostMaster::getFormattedPostText(const ATProto::AppBskyFeed::Record::Post& post, const QString& linkColor)
{
    if (post.mFacets.empty())
        return ATProto::PostMaster::plainToHtml(post.mText);
    else
        return ATProto::AppBskyRichtext::applyFacets(post.mText, post.mFacets, linkColor);
}

PostMaster::PostMaster(Client& client) :
    Presence(),
    mClient(client)
{
}

void PostMaster::post(const ATProto::AppBskyFeed::Record::Post& post,
                      const PostSuccessCb& successCb, const Client::ErrorCb& errorCb)
{
    QJsonObject postJson;

    try {
        postJson = post.toJson();
    } catch (InvalidContent& e) {
        if (errorCb)
            QTimer::singleShot(0, &mPresence, [errorCb, e]{ errorCb("InvalidContent", "Invalid content: " + e.msg()); });
    }

    qDebug() << "Posting:" << postJson;
    const QString& repo = mClient.getSession()->mDid;
    const QString collection = postJson["$type"].toString();

    mClient.createRecord(repo, collection, {}, postJson,
        [successCb](auto strongRef){
            if (successCb)
                successCb(strongRef->mUri, strongRef->mCid);
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

void PostMaster::addThreadgate(const QString& uri, bool allowMention, bool allowFollowing,
                   const Client::SuccessCb& successCb, const Client::ErrorCb& errorCb)
{
    const auto atUri = ATUri::createAtUri(uri, mPresence, errorCb);
    if (!atUri.isValid())
        return;

    ATProto::AppBskyFeed::Threadgate threadgate;
    threadgate.mPost = uri;
    threadgate.mAllowMention = allowMention;
    threadgate.mAllowFollowing = allowFollowing;
    threadgate.mCreatedAt = QDateTime::currentDateTimeUtc();

    QJsonObject threadgateJson = threadgate.toJson();
    qDebug() << "Add threadgate:" << threadgateJson;
    const QString& repo = mClient.getSession()->mDid;
    const QString collection = threadgateJson["$type"].toString();

    mClient.createRecord(repo, collection, atUri.getRkey(), threadgateJson,
        [successCb](auto){
            if (successCb)
                successCb();
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

void PostMaster::repost(const QString& uri, const QString& cid,
            const RepostSuccessCb& successCb, const Client::ErrorCb& errorCb)
{
    const auto atUri = ATUri::createAtUri(uri, mPresence, errorCb);
    if (!atUri.isValid())
        return;

    AppBskyFeed::Repost repost;
    repost.mSubject = std::make_unique<ComATProtoRepo::StrongRef>();
    repost.mSubject->mUri = uri;
    repost.mSubject->mCid = cid;
    repost.mCreatedAt = QDateTime::currentDateTimeUtc();

    const auto repostJson = repost.toJson();
    const QString& repo = mClient.getSession()->mDid;
    const QString collection = repostJson["$type"].toString();

    mClient.createRecord(repo, collection, {}, repostJson,
        [successCb](auto strongRef){
            if (successCb)
                successCb(strongRef->mUri, strongRef->mCid);
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

void PostMaster::like(const QString& uri, const QString& cid,
          const LikeSuccessCb& successCb, const Client::ErrorCb& errorCb)
{
    const auto atUri = ATUri::createAtUri(uri, mPresence, errorCb);
    if (!atUri.isValid())
        return;

    AppBskyFeed::Like like;
    like.mSubject = std::make_unique<ComATProtoRepo::StrongRef>();
    like.mSubject->mUri = uri;
    like.mSubject->mCid = cid;
    like.mCreatedAt = QDateTime::currentDateTimeUtc();

    const auto likeJson = like.toJson();
    const QString& repo = mClient.getSession()->mDid;
    const QString collection = likeJson["$type"].toString();

    mClient.createRecord(repo, collection, {}, likeJson,
        [successCb](auto strongRef){
            if (successCb)
                successCb(strongRef->mUri, strongRef->mCid);
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

void PostMaster::undo(const QString& uri,
                      const Client::SuccessCb& successCb, const Client::ErrorCb& errorCb)
{
    qDebug() << "Undo:" << uri;
    const auto atUri = ATUri::createAtUri(uri, mPresence, errorCb);
    if (!atUri.isValid())
        return;

    mClient.deleteRecord(atUri.getAuthority(), atUri.getCollection(), atUri.getRkey(),
        [successCb]{
            if (successCb)
                successCb();
        },
        [errorCb](const QString& err, const QString& msg) {
            if (errorCb)
                errorCb(err, msg);
        });
}

void PostMaster::checkPostExists(const QString& uri, const QString& cid,
                                 const Client::SuccessCb& successCb, const Client::ErrorCb& errorCb)
{
    const auto atUri = ATUri::createAtUri(uri, mPresence, errorCb);
    if (!atUri.isValid())
        return;

    mClient.getRecord(atUri.getAuthority(), atUri.getCollection(), atUri.getRkey(), cid,
        [successCb](auto) {
            if (successCb)
                successCb();
        },
        [errorCb](const QString& err, const QString& msg) {
            if (errorCb)
                errorCb(err, msg);
        });
}

void PostMaster::getPost(const QString& httpsUri, const PostCb& successCb)
{
    auto atUri = ATUri::fromHttpsPostUri(httpsUri);

    if (!atUri.isValid())
        return;

    mClient.getProfile(atUri.getAuthority(),
        [this, presence=getPresence(), atUri, successCb](auto profile){
            if (!presence)
                return;

            ATUri newUri(atUri);
            newUri.setAuthority(profile->mDid);
            newUri.setAuthorityIsHandle(false);
            continueGetPost(newUri, std::move(profile), successCb);
        },
        [](const QString& err, const QString& msg){
            qDebug() << err << " - " << msg;
        });
}

void PostMaster::continueGetPost(const ATUri& atUri, AppBskyActor::ProfileViewDetailed::Ptr author, const PostCb& successCb)
{
    auto newAuthor = AppBskyActor::ProfileViewDetailed::SharedPtr(author.release());

    mClient.getRecord(atUri.getAuthority(), atUri.getCollection(), atUri.getRkey(), {},
        [successCb, newAuthor](ComATProtoRepo::Record::Ptr record){
            try {
                auto post = AppBskyFeed::Record::Post::fromJson(record->mValue);

                if (successCb)
                    successCb(record->mUri, record->mCid.value_or(""), std::move(post), newAuthor);
            } catch (InvalidJsonException& e) {
                qWarning() << e.msg();
            }
        },
        [](const QString& err, const QString& msg){
            qDebug() << err << " - " << msg;
        });
}

void PostMaster::createPost(const QString& text, AppBskyFeed::PostReplyRef::Ptr replyRef, const PostCreatedCb& cb)
{
    Q_ASSERT(cb);
    auto post = std::make_shared<ATProto::AppBskyFeed::Record::Post>();
    post->mText = text;
    post->mCreatedAt = QDateTime::currentDateTimeUtc();
    post->mReply = std::move(replyRef);

    auto facets = parseFacets(text);
    resolveFacets(post, facets, 0, cb);
}

void PostMaster::resolveFacets(AppBskyFeed::Record::Post::SharedPtr post,
                           std::vector<ParsedMatch> facets, int facetIndex,
                           const PostCreatedCb& cb)
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
                [this, presence=getPresence(), i, post, facets, cb](const QString& did){
                    if (!presence)
                        return;

                    auto newFacets = facets;
                    newFacets[i].mRef = did;
                    resolveFacets(post, newFacets, i + 1, cb);
                },
                [this, presence=getPresence(), i, post, facets, cb](const QString& error, const QString& msg){
                    if (!presence)
                        return;

                    qWarning() << "Could not resolve handle:" << error << " - " << msg << "match:" << facets[i].mMatch;
                    resolveFacets(post, facets, i + 1, cb);
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

    addFacets(post, facets);
    cb(post);
}

QString PostMaster::shortenWebLink(const QString& link)
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

static int convertIndextoUtf8Index(int index, const QString& str)
{
    qDebug() << "UTF-8:" << str.sliced(0, index).toUtf8();
    return str.sliced(0, index).toUtf8().length();
}

void PostMaster::addFacets(AppBskyFeed::Record::Post::SharedPtr post,
                           const std::vector<ParsedMatch>& facets)
{
    int pos = 0;
    QString shortenedText;

    for (const auto& f : facets)
    {
        if (f.mRef.isEmpty())
            continue;

        shortenedText += post->mText.sliced(pos, f.mStartIndex - pos);
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
        post->mFacets.push_back(std::move(facet));
    }

    if (pos < post->mText.size())
        shortenedText += post->mText.sliced(pos);

    post->mText = shortenedText;
}

void PostMaster::addQuoteToPost(AppBskyFeed::Record::Post& post, const QString& quoteUri, const QString& quoteCid)
{

    auto ref = std::make_unique<ComATProtoRepo::StrongRef>();
    ref->mUri = quoteUri;
    ref->mCid = quoteCid;

    Q_ASSERT(!post.mEmbed);
    post.mEmbed = std::make_unique<AppBskyEmbed::Embed>();
    post.mEmbed->mType = AppBskyEmbed::EmbedType::RECORD;
    post.mEmbed->mEmbed = std::make_unique<AppBskyEmbed::Record>();

    auto& record = std::get<AppBskyEmbed::Record::Ptr>(post.mEmbed->mEmbed);
    record->mRecord = std::move(ref);
}

void PostMaster::addImageToPost(AppBskyFeed::Record::Post& post, Blob::Ptr blob, const QString& altText)
{
    if (!post.mEmbed)
    {
        post.mEmbed = std::make_unique<AppBskyEmbed::Embed>();
        post.mEmbed->mType = AppBskyEmbed::EmbedType::IMAGES;
        post.mEmbed->mEmbed = std::make_unique<AppBskyEmbed::Images>();
    }
    else if (post.mEmbed->mType == AppBskyEmbed::EmbedType::RECORD)
    {
        auto& record = std::get<AppBskyEmbed::Record::Ptr>(post.mEmbed->mEmbed);
        auto ref = std::move(record->mRecord);
        post.mEmbed->mType = AppBskyEmbed::EmbedType::RECORD_WITH_MEDIA;
        post.mEmbed->mEmbed = std::make_unique<AppBskyEmbed::RecordWithMedia>();

        auto& recordWithMedia = std::get<AppBskyEmbed::RecordWithMedia::Ptr>(post.mEmbed->mEmbed);
        recordWithMedia->mRecord = std::make_unique<AppBskyEmbed::Record>();
        recordWithMedia->mRecord->mRecord = std::move(ref);
        recordWithMedia->mMediaType = AppBskyEmbed::EmbedType::IMAGES;
        recordWithMedia->mMedia = std::make_unique<AppBskyEmbed::Images>();
    }

    auto image = std::make_unique<AppBskyEmbed::Image>();
    image->mImage = std::move(blob);
    image->mAlt = altText;

    AppBskyEmbed::Images* images = nullptr;
    if (post.mEmbed->mType == AppBskyEmbed::EmbedType::IMAGES)
    {
        images = std::get<AppBskyEmbed::Images::Ptr>(post.mEmbed->mEmbed).get();
    }
    else
    {
        Q_ASSERT(post.mEmbed->mType == AppBskyEmbed::EmbedType::RECORD_WITH_MEDIA);
        auto& recordWithMedia = std::get<AppBskyEmbed::RecordWithMedia::Ptr>(post.mEmbed->mEmbed);
        images = std::get<AppBskyEmbed::Images::Ptr>(recordWithMedia->mMedia).get();
    }

    Q_ASSERT(images);
    images->mImages.push_back(std::move(image));
}

void PostMaster::addExternalToPost(AppBskyFeed::Record::Post& post, const QString& link,
                               const QString& title, const QString& description, Blob::Ptr blob)
{
    AppBskyEmbed::External* embed = nullptr;

    if (!post.mEmbed)
    {
        post.mEmbed = std::make_unique<AppBskyEmbed::Embed>();
        post.mEmbed->mType = AppBskyEmbed::EmbedType::EXTERNAL;
        post.mEmbed->mEmbed = std::make_unique<AppBskyEmbed::External>();
        embed = std::get<AppBskyEmbed::External::Ptr>(post.mEmbed->mEmbed).get();
    }
    else
    {
        Q_ASSERT(post.mEmbed->mType == AppBskyEmbed::EmbedType::RECORD);
        auto& record = std::get<AppBskyEmbed::Record::Ptr>(post.mEmbed->mEmbed);
        auto ref = std::move(record->mRecord);
        post.mEmbed->mType = AppBskyEmbed::EmbedType::RECORD_WITH_MEDIA;
        post.mEmbed->mEmbed = std::make_unique<AppBskyEmbed::RecordWithMedia>();

        auto& recordWithMedia = std::get<AppBskyEmbed::RecordWithMedia::Ptr>(post.mEmbed->mEmbed);
        recordWithMedia->mRecord = std::make_unique<AppBskyEmbed::Record>();
        recordWithMedia->mRecord->mRecord = std::move(ref);
        recordWithMedia->mMediaType = AppBskyEmbed::EmbedType::EXTERNAL;
        recordWithMedia->mMedia = std::make_unique<AppBskyEmbed::External>();

        embed = std::get<AppBskyEmbed::External::Ptr>(recordWithMedia->mMedia).get();
    }

    Q_ASSERT(embed);
    embed->mExternal = std::make_unique<AppBskyEmbed::ExternalExternal>();
    embed->mExternal->mUri = link;
    embed->mExternal->mTitle = title;
    embed->mExternal->mDescription = description;
    embed->mExternal->mThumb = std::move(blob);
}

static std::vector<PostMaster::ParsedMatch> parseMatches(PostMaster::ParsedMatch::Type type, const QString& text, const QRegularExpression& re, int group)
{
    std::vector<PostMaster::ParsedMatch> matches;

    // HACK: prefixing a space to match a mention at the start of the text
    for (const auto& match : re.globalMatch(' ' + text))
    {
        PostMaster::ParsedMatch parsedMatch;
        parsedMatch.mMatch = match.captured(group);
        parsedMatch.mStartIndex = match.capturedStart(group) - 1; // -1 for the hack
        parsedMatch.mEndIndex = match.capturedEnd(group) - 1;
        parsedMatch.mType = type;
        matches.push_back(parsedMatch);
    }

    return matches;
}

std::vector<PostMaster::ParsedMatch> PostMaster::parseTags(const QString& text)
{
    static const QRegularExpression reTag(R"([$|\W](#\w+))");
    const auto tags = parseMatches(ParsedMatch::Type::TAG, text, reTag, 1);

    for (const auto& tag : tags)
        qDebug() << "Tag:" << tag.mMatch << "start:" << tag.mStartIndex << "end:" << tag.mEndIndex;

    return tags;
}

std::vector<PostMaster::ParsedMatch> PostMaster::parsePartialMentions(const QString& text)
{
    static const QRegularExpression rePartialMention(R"([$|\W](@[a-zA-Z0-9]([a-zA-Z0-9-]{0,61}[a-zA-Z0-9])?))");
    const auto partialMentions = parseMatches(ParsedMatch::Type::PARTIAL_MENTION, text, rePartialMention, 1);

    for (const auto& partialMention : partialMentions)
        qDebug() << "Partial mention:" << partialMention.mMatch << "start:" << partialMention.mStartIndex << "end:" << partialMention.mEndIndex;

    return partialMentions;
}

std::vector<PostMaster::ParsedMatch> PostMaster::parseMentions(const QString& text)
{
    static const QRegularExpression reMention(QString(R"([$|\W](@%1))").arg(ATRegex::HANDLE.pattern()));
    const auto mentions = parseMatches(ParsedMatch::Type::MENTION, text, reMention, 1);

    for (const auto& mention : mentions)
        qDebug() << "Mention:" << mention.mMatch << "start:" << mention.mStartIndex << "end:" << mention.mEndIndex;

    return mentions;
}

std::vector<PostMaster::ParsedMatch> PostMaster::parseLinks(const QString& text)
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

static void addToSortedMatches(std::map<int, PostMaster::ParsedMatch>& sortedMatches,
                               const std::vector<PostMaster::ParsedMatch>& matches)
{
    for (const auto& match : matches)
    {
        if (sortedMatches.count(match.mStartIndex))
        {
            Q_ASSERT(match.mType == PostMaster::ParsedMatch::Type::MENTION &&
                     sortedMatches[match.mStartIndex].mType == PostMaster::ParsedMatch::Type::PARTIAL_MENTION);
            qDebug() << "Two matches with start index:" << match.mStartIndex
                     << match.mMatch << sortedMatches[match.mStartIndex].mMatch;
        }

        sortedMatches[match.mStartIndex] = match;
    }
}

std::vector<PostMaster::ParsedMatch> PostMaster::parseFacets(const QString& text)
{
    const auto tags = parseTags(text);
    const auto partialMentions = parsePartialMentions(text);
    const auto mentions = parseMentions(text);
    const auto links = parseLinks(text);
    std::vector<PostMaster::ParsedMatch> facets;
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
            Q_ASSERT(match.mType == PostMaster::ParsedMatch::Type::PARTIAL_MENTION);
            qWarning() << "Overlapping facets at index:" << match.mStartIndex << match.mMatch;
            continue;
        }

        facets.push_back(match);
        pos = match.mEndIndex;
    }

    return facets;
}

std::vector<QString> PostMaster::getFacetTags(AppBskyFeed::Record::Post& post)
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
