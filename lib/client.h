// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "xjson.h"
#include "xrpc_client.h"
#include "lexicon/app_bsky_actor.h"
#include "lexicon/app_bsky_feed.h"
#include "lexicon/app_bsky_graph.h"
#include "lexicon/com_atproto_server.h"
#include <QException>

namespace ATProto {

class InvalidRequest : public QException
{
public:
    explicit InvalidRequest(const QString& msg) : mMsg(msg) {}

    const QString& msg() const { return mMsg; }
    void raise() const override { throw *this; }
    InvalidRequest *clone() const override { return new InvalidRequest(*this); }

private:
    QString mMsg;
};

class Client
{
public:
    using SuccessCb = std::function<void()>;
    using GetProfileSuccessCb = std::function<void(AppBskyActor::ProfileViewDetailed::Ptr)>;
    using GetAuthorFeedSuccessCb = std::function<void(AppBskyFeed::OutputFeed::Ptr)>;
    using GetTimelineSuccessCb = std::function<void(AppBskyFeed::OutputFeed::Ptr)>;
    using GetPostThreadSuccessCb = std::function<void(AppBskyFeed::PostThread::Ptr)>;
    using GetFollowsSuccessCb = std::function<void(AppBskyGraph::GetFollowsOutput::Ptr)>;
    using UploadBlobSuccessCb = std::function<void(ATProto::Blob::Ptr)>;
    using ErrorCb = std::function<void(const QString& err)>;

    explicit Client(std::unique_ptr<Xrpc::Client>&& xrpc);

    const QString& getHost() const { return mXrpc->getHost(); }
    const ComATProtoServer::Session* getSession() const { return mSession.get(); }

    // com.atproto.server
    /**
     * @brief createSession
     * @param user user handle or did
     * @param pwd password
     * @param successCb
     * @param errorCb
     */
    void createSession(const QString& user, const QString& pwd,
                       const SuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief resumeSession Resume a previously created session
     * @param session
     * @param successCb
     * @param errorCb
     */
    void resumeSession(const ComATProtoServer::Session& session,
                       const SuccessCb& successCb, const ErrorCb& errorCb);

    void refreshSession(const ComATProtoServer::Session& session,
                        const SuccessCb& successCb, const ErrorCb& errorCb);

    // com.bsky.actor
    /**
     * @brief getProfile
     * @param user user handle or did
     * @param successCb
     * @param errorCb
     */
    void getProfile(const QString& user, const GetProfileSuccessCb& successCb, const ErrorCb& errorCb);

    // app.bsky.feed
    /**
     * @brief getAuthorFeed
     * @param user user handle or did
     * @param limit min=1 max=100 default=50
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getAuthorFeed(const QString& user, std::optional<int> limit, const std::optional<QString>& cursor,
                       const GetAuthorFeedSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getTimeline
     * @param limit min=1 max=100 default=50
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getTimeline(std::optional<int> limit, const std::optional<QString>& cursor,
                     const GetTimelineSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getPostThread
     * @param uri
     * @param depth min=0 max=1000 default=6
     * @param parentHeight min=0 max=1000 default=80
     * @param successCb
     * @param errorCb
     */
    void getPostThread(const QString& uri, std::optional<int> depth, std::optional<int> parentHeight,
                       const GetPostThreadSuccessCb& successCb, const ErrorCb& errorCb);

    // app.bsky.graph

    /**
     * @brief getFollows
     * @param actor
     * @param limit min=1 default=50 max=100
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getFollows(const QString& actor, std::optional<int> limit, const std::optional<QString>& cursor,
                    const GetFollowsSuccessCb& successCb, const ErrorCb& errorCb);

    void uploadBlob(const QByteArray& blob, const QString& mimeType,
                    const UploadBlobSuccessCb& successCb, const ErrorCb& errorCb);

    void post(const ATProto::AppBskyFeed::Record::Post& post,
              const SuccessCb& successCb, const ErrorCb& errorCb);

    static ATProto::AppBskyFeed::Record::Post::SharedPtr createPost(const QString& text);
    static void addImageToPost(ATProto::AppBskyFeed::Record::Post& post, ATProto::Blob::Ptr blob);

private:
    const QString& authToken() const;
    const QString& refreshToken() const;

    // Create XRPC error callback from ATProto client callback
    Xrpc::Client::ErrorCb failure(const ErrorCb& cb);

    void invalidJsonError(InvalidJsonException& e, const ErrorCb& cb);
    void requestFailed(const QString& err, const QJsonDocument& json, const ErrorCb& errorCb);

    std::unique_ptr<Xrpc::Client> mXrpc;
    ComATProtoServer::Session::Ptr mSession;
};

}
