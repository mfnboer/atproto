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
    using ThreadgateSuccessCb = std::function<void(const QString& uri, const QString& cid)>;
    using PostgateSuccessCb = std::function<void(const QString& uri, const QString& cid)>;
    using EmbeddingDetachedCb = std::function<void(const QString& uri, const QString& cid, bool detached)>;
    using RepostSuccessCb = std::function<void(const QString& uri, const QString& cid)>;
    using LikeSuccessCb = std::function<void(const QString& uri, const QString& cid)>;
    using PostCb = std::function<void(const QString& uri, const QString& cid, AppBskyFeed::Record::Post::SharedPtr, AppBskyActor::ProfileViewDetailed::SharedPtr)>;
    using FeedCb = std::function<void(AppBskyFeed::GeneratorView::SharedPtr)>;
    using ListCb = std::function<void(AppBskyGraph::ListView::SharedPtr)>;
    using PostgateCb = std::function<void(AppBskyFeed::Postgate::SharedPtr)>;
    using SuccessCb = Client::SuccessCb;
    using ErrorCb = Client::ErrorCb;
    using ProgressCb = std::function<void(const QString& status, std::optional<int> progress)>;

    explicit PostMaster(Client& client);

    void post(const ATProto::AppBskyFeed::Record::Post& post,
              const PostSuccessCb& successCb, const ErrorCb& errorCb);
    void addThreadgate(const QString& uri, bool allowMention, bool allowFollower, bool allowFollowing, const QStringList& allowLists,
                       bool allowNobody, const QStringList& hiddenReplies,
                       const ThreadgateSuccessCb& successCb, const ErrorCb& errorCb);
    void addPostgate(const QString& uri, bool disableEmbedding, const QStringList& detachedEmbeddingUris,
                     const PostgateSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @param detach true -> detach, false -> re-attach
     */
    void detachEmbedding(const QString& uri, const QString& embeddingUri, const QString& embeddingCid,
                         bool detach, const EmbeddingDetachedCb& successCb, const ErrorCb& errorCb);

    void continueDetachEmbedding(const QString& uri, const QString& embeddingUri, const QString& embeddingCid,
                                 const std::vector<QString>& currentDetachedEmbeddingUris, bool detach,
                                 const EmbeddingDetachedCb& successCb, const ErrorCb& errorCb);
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
    void continueGetPost(const ATUri& atUri, AppBskyActor::ProfileViewDetailed::SharedPtr author, const PostCb& successCb);
    void getFeed(const QString& httpsUri, const FeedCb& successCb);
    void continueGetFeed(const ATUri& atUri, const FeedCb& successCb);
    void getList(const QString& httpsUri, const ListCb& successCb);
    void continueGetList(const ATUri& atUri, const ListCb& successCb);

    void getPostgate(const QString& postUri, const PostgateCb& successCb, const ErrorCb& errorCb);

    static AppBskyFeed::Threadgate::SharedPtr createThreadgate(const QString& uri, bool allowMention, bool allowFollower,
            bool allowFollowing, const QStringList& allowLists, bool allowNobody, const QStringList& hiddenReplies);

    static AppBskyFeed::Postgate::SharedPtr createPostgate(const QString& uri, bool disableEmbedding, const QStringList& detachedEmbeddingUris);
    QString createPostgateUri(const QString postUri);

    static AppBskyFeed::PostReplyRef::SharedPtr createReplyRef(
            const QString& replyToUri, const QString& replyToCid,
            const QString& replyRootUri, const QString& replyRootCid);
    static AppBskyFeed::Record::Post::SharedPtr createPostWithoutFacets(
        const QString& text, const QString& language, AppBskyFeed::PostReplyRef::SharedPtr replyRef);

    void createPost(const QString& text, const QString& language,
                    AppBskyFeed::PostReplyRef::SharedPtr replyRef,
                    const std::vector<RichTextMaster::ParsedMatch>& embeddedLinks,
                    const PostCreatedCb& cb);
    static void addQuoteToPost(AppBskyFeed::Record::Post& post, const QString& quoteUri, const QString& quoteCid);
    static void addLabelsToPost(AppBskyFeed::Record::Post& post, const QStringList& labels);
    static void addImageToPost(AppBskyFeed::Record::Post& post, Blob::SharedPtr blob, int width, int height, const QString& altText);
    static void addExternalToPost(AppBskyFeed::Record::Post& post, const QString& link,
                                  const QString& title, const QString& description, Blob::SharedPtr blob = nullptr);

    static void addVideoToPost(AppBskyFeed::Record::Post& post, Blob::SharedPtr blob, int width, int height, const QString& altText);
    void addVideoToPost(AppBskyFeed::Record::Post::SharedPtr post, const AppBskyVideo::JobStatus& jobStatus, int width, int height, const QString& altText,
                        const SuccessCb& successCb, const ErrorCb& errorCb, const ProgressCb& progressCb);
    void checkVideoUploadStatus(AppBskyFeed::Record::Post::SharedPtr post, const QString jobId, int width, int height, const QString& altText,
                                const SuccessCb& successCb, const ErrorCb& errorCb, const ProgressCb& progressCb);

    void sendInteractionShowMoreLikeThis(const QString& postUri, const QString& feedDid, const QString& feedContext,
                                         const SuccessCb& successCb, const ErrorCb& errorCb);
    void sendInteractionShowLessLikeThis(const QString& postUri, const QString& feedDid, const QString& feedContext,
                                         const SuccessCb& successCb, const ErrorCb& errorCb);

private:
    void sendInteraction(const QString& postUri, const QString& feedDid, const QString& feedContext,
                         AppBskyFeed::Interaction::EventType event, const SuccessCb& successCb, const ErrorCb& errorCb);

    Client& mClient;
    RichTextMaster mRichTextMaster;
    QObject mPresence;
};

}
