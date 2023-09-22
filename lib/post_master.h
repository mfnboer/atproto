// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "at_uri.h"
#include "client.h"

namespace ATProto {

/**
 * @brief Functions to compose and send posts.
 */
class PostMaster
{
public:
    using PostCreatedCb = std::function<void(AppBskyFeed::Record::Post::SharedPtr)>;

    struct ParsedMatch
    {
        using Type = ATProto::AppBskyRichtext::Facet::Feature::Type;

        int mStartIndex;
        int mEndIndex;
        QString mMatch;
        Type mType;
        QString mRef;
    };

    explicit PostMaster(Client& client);

    void post(const ATProto::AppBskyFeed::Record::Post& post,
              const Client::SuccessCb& successCb, const Client::ErrorCb& errorCb);
    void repost(const QString& uri, const QString& cid,
                const Client::SuccessCb& successCb, const Client::ErrorCb& errorCb);
    void undoRepost(const QString& uri,
                    const Client::SuccessCb& successCb, const Client::ErrorCb& errorCb);

    void checkPostExists(const QString& uri, const QString& cid,
                         const Client::SuccessCb& successCb, const Client::ErrorCb& errorCb);


    void createPost(const QString& text, AppBskyFeed::PostReplyRef::Ptr replyRef, const PostCreatedCb& cb);

    void resolveFacets(AppBskyFeed::Record::Post::SharedPtr post,
                       std::vector<ParsedMatch> facets, int facetIndex,
                       const PostCreatedCb& cb);
    void addFacets(AppBskyFeed::Record::Post::SharedPtr post,
                   const std::vector<ParsedMatch>& facets);

    void addQuoteToPost(AppBskyFeed::Record::Post& post, const QString& quoteUri, const QString& quoteCid);

    static QString shortenWebLink(const QString& link);

    static void addImageToPost(AppBskyFeed::Record::Post& post, Blob::Ptr blob);
    static void addExternalToPost(AppBskyFeed::Record::Post& post, const QString& link,
                                  const QString& title, const QString& description, Blob::Ptr blob = nullptr);

    static std::vector<ParsedMatch> parseMentions(const QString& text);
    static std::vector<ParsedMatch> parseLinks(const QString& text);

    // If two facets overlap, then the one with the lowest start index is taken
    static std::vector<ParsedMatch> parseFacets(const QString& text);

private:
    ATUri createAtUri(const QString& uri, const Client::ErrorCb& errorCb) const;

    Client& mClient;
    QObject mPresence;
};

}
