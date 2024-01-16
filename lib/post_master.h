// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "at_uri.h"
#include "client.h"
#include "presence.h"
#include "rich_text_master.h"

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
    using FeedCb = std::function<void(AppBskyFeed::GeneratorView::Ptr)>;
    using ListCb = std::function<void(AppBskyGraph::ListView::Ptr)>;
    using ErrorCb = Client::ErrorCb;

    explicit PostMaster(Client& client);

    void post(const ATProto::AppBskyFeed::Record::Post& post,
              const PostSuccessCb& successCb, const ErrorCb& errorCb);
    void addThreadgate(const QString& uri, bool allowMention, bool allowFollowing,
                       const Client::SuccessCb& successCb, const ErrorCb& errorCb);
    void repost(const QString& uri, const QString& cid,
                const RepostSuccessCb& successCb, const ErrorCb& errorCb);
    void like(const QString& uri, const QString& cid,
              const LikeSuccessCb& successCb, const ErrorCb& errorCb);
    void undo(const QString& uri,
              const Client::SuccessCb& successCb, const ErrorCb& errorCb);

    // A record can be a post record or a generator record
    void checkRecordExists(const QString& uri, const QString& cid,
                         const Client::SuccessCb& successCb, const ErrorCb& errorCb);

    void getPost(const QString& httpsUri, const PostCb& successCb);
    void continueGetPost(const ATUri& atUri, AppBskyActor::ProfileViewDetailed::Ptr author, const PostCb& successCb);
    void getFeed(const QString& httpsUri, const FeedCb& successCb);
    void continueGetFeed(const ATUri& atUri, const FeedCb& successCb);
    void getList(const QString& httpsUri, const ListCb& successCb);
    void continueGetList(const ATUri& atUri, const ListCb& successCb);

    void createPost(const QString& text, AppBskyFeed::PostReplyRef::Ptr replyRef, const PostCreatedCb& cb);
    void addQuoteToPost(AppBskyFeed::Record::Post& post, const QString& quoteUri, const QString& quoteCid);
    static void addImageToPost(AppBskyFeed::Record::Post& post, Blob::Ptr blob, const QString& altText);
    static void addExternalToPost(AppBskyFeed::Record::Post& post, const QString& link,
                                  const QString& title, const QString& description, Blob::Ptr blob = nullptr);

private:
    Client& mClient;
    RichTextMaster mRichTextMaster;
    QObject mPresence;
};

}
