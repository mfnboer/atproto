// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "at_uri.h"
#include "client.h"
#include "presence.h"

namespace ATProto {

/**
 * @brief Functions to compose, send and like posts.
 */
class PostMaster : public Presence
{
public:
    using PostCreatedCb = std::function<void(AppBskyFeed::Record::Post::SharedPtr)>;
    using PostSuccessCb = std::function<void(const QString& uri, const QString& cid)>;
    using RepostSuccessCb = std::function<void(const QString& uri, const QString& cid)>;
    using LikeSuccessCb = std::function<void(const QString& uri, const QString& cid)>;
    using PostCb = std::function<void(const QString& uri, const QString& cid, AppBskyFeed::Record::Post::Ptr, AppBskyActor::ProfileViewDetailed::SharedPtr)>;

    struct ParsedMatch
    {
        using Type = ATProto::AppBskyRichtext::Facet::Feature::Type;

        int mStartIndex;
        int mEndIndex;
        QString mMatch;
        Type mType;
        QString mRef;
    };

    static QString plainToHtml(const QString& text);
    static QString getFormattedPostText(const ATProto::AppBskyFeed::Record::Post& post, const QString& linkColor = "");

    explicit PostMaster(Client& client);

    void post(const ATProto::AppBskyFeed::Record::Post& post,
              const PostSuccessCb& successCb, const Client::ErrorCb& errorCb);
    void addThreadgate(const QString& uri, bool allowMention, bool allowFollowing,
                       const Client::SuccessCb& successCb, const Client::ErrorCb& errorCb);
    void repost(const QString& uri, const QString& cid,
                const RepostSuccessCb& successCb, const Client::ErrorCb& errorCb);
    void like(const QString& uri, const QString& cid,
              const LikeSuccessCb& successCb, const Client::ErrorCb& errorCb);
    void undo(const QString& uri,
              const Client::SuccessCb& successCb, const Client::ErrorCb& errorCb);

    void checkPostExists(const QString& uri, const QString& cid,
                         const Client::SuccessCb& successCb, const Client::ErrorCb& errorCb);

    void getPost(const QString& httpsUri, const PostCb& successCb);
    void continueGetPost(const ATUri& atUri, AppBskyActor::ProfileViewDetailed::Ptr author, const PostCb& successCb);

    void createPost(const QString& text, AppBskyFeed::PostReplyRef::Ptr replyRef, const PostCreatedCb& cb);

    void resolveFacets(AppBskyFeed::Record::Post::SharedPtr post,
                       std::vector<ParsedMatch> facets, int facetIndex,
                       const PostCreatedCb& cb);
    void addFacets(AppBskyFeed::Record::Post::SharedPtr post,
                   const std::vector<ParsedMatch>& facets);

    void addQuoteToPost(AppBskyFeed::Record::Post& post, const QString& quoteUri, const QString& quoteCid);

    static QString shortenWebLink(const QString& link);

    static void addImageToPost(AppBskyFeed::Record::Post& post, Blob::Ptr blob, const QString& altText);
    static void addExternalToPost(AppBskyFeed::Record::Post& post, const QString& link,
                                  const QString& title, const QString& description, Blob::Ptr blob = nullptr);

    static std::vector<ParsedMatch> parsePartialMentions(const QString& text);
    static std::vector<ParsedMatch> parseMentions(const QString& text);
    static std::vector<ParsedMatch> parseLinks(const QString& text);
    static std::vector<ParsedMatch> parseTags(const QString& text);

    // If two facets overlap, then the one with the lowest start index is taken
    static std::vector<ParsedMatch> parseFacets(const QString& text);

    static std::vector<QString> getFacetTags(AppBskyFeed::Record::Post& post);

private:
    Client& mClient;
    QObject mPresence;
};

}
