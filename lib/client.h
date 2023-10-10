// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "user_preferences.h"
#include "xjson.h"
#include "xrpc_client.h"
#include "lexicon/app_bsky_actor.h"
#include "lexicon/app_bsky_feed.h"
#include "lexicon/app_bsky_graph.h"
#include "lexicon/app_bsky_notification.h"
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
    using ResolveHandleSuccessCb = std::function<void(QString)>;
    using GetProfileSuccessCb = std::function<void(AppBskyActor::ProfileViewDetailed::Ptr)>;
    using GetAuthorFeedSuccessCb = std::function<void(AppBskyFeed::OutputFeed::Ptr)>;
    using GetTimelineSuccessCb = std::function<void(AppBskyFeed::OutputFeed::Ptr)>;
    using GetPostThreadSuccessCb = std::function<void(AppBskyFeed::PostThread::Ptr)>;
    using GetPostsSuccessCb = std::function<void(AppBskyFeed::PostViewList)>;
    using GetLikesSuccessCb = std::function<void(AppBskyFeed::GetLikesOutput::Ptr)>;
    using GetRepostedBySuccessCb = std::function<void(AppBskyFeed::GetRepostedByOutput::Ptr)>;
    using GetFollowsSuccessCb = std::function<void(AppBskyGraph::GetFollowsOutput::Ptr)>;
    using GetFollowersSuccessCb = std::function<void(AppBskyGraph::GetFollowersOutput::Ptr)>;
    using UploadBlobSuccessCb = std::function<void(Blob::Ptr)>;
    using GetRecordSuccessCb = std::function<void(ComATProtoRepo::Record::Ptr)>;
    using CreateRecordSuccessCb = std::function<void(ComATProtoRepo::StrongRef::Ptr)>;
    using UnreadCountSuccessCb = std::function<void(int)>;
    using NotificationsSuccessCb = std::function<void(AppBskyNotification::ListNotificationsOutput::Ptr)>;
    using UserPrefsSuccessCb = std::function<void(UserPreferences)>;
    using ErrorCb = std::function<void(const QString& err)>;

    static constexpr int MAX_URIS_GET_POSTS = 25;

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

    // com.atproto.identity
    /**
     * @brief resolveHandle Resolve a handle to a DID
     * @param handle
     * @param successCb
     * @param errorCb
     */
    void resolveHandle(const QString& handle,
                       const ResolveHandleSuccessCb& successCb, const ErrorCb& errorCb);

    // app.bsky.actor
    /**
     * @brief getProfile
     * @param user user handle or did
     * @param successCb
     * @param errorCb
     */
    void getProfile(const QString& user, const GetProfileSuccessCb& successCb, const ErrorCb& errorCb);

    void getPreferences(const UserPrefsSuccessCb& successCb, const ErrorCb& errorCb);
    void putPreferences(const UserPreferences& userPrefs,
                        const SuccessCb& successCb, const ErrorCb& errorCb);

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

    /**
     * @brief getPosts
     * @param uris max 25 at-uri's
     * @param successCb
     * @param errorCb
     */
    void getPosts(const std::vector<QString>& uris,
                  const GetPostsSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getLikes
     * @param uri
     * @param limit min=1 max=100 default=50
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getLikes(const QString& uri, std::optional<int> limit, const std::optional<QString>& cursor,
                  const GetLikesSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getRepostedBy
     * @param uri
     * @param limit min=1 max=100 default=50
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getRepostedBy(const QString& uri, std::optional<int> limit, const std::optional<QString>& cursor,
                       const GetRepostedBySuccessCb& successCb, const ErrorCb& errorCb);

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

    /**
     * @brief getFollowers
     * @param actor
     * @param limit min=1 default=50 max=100
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getFollowers(const QString& actor, std::optional<int> limit, const std::optional<QString>& cursor,
                      const GetFollowersSuccessCb& successCb, const ErrorCb& errorCb);

    void muteActor(const QString& actor, const SuccessCb& successCb, const ErrorCb& errorCb);
    void unmuteActor(const QString& actor, const SuccessCb& successCb, const ErrorCb& errorCb);


    // app.bsky.notification

    /**
     * @brief getUnreadCount Get the number of unread notifications since last time.
     * @param seenAt Last timestamp the notifications were seen
     * @param successCb
     * @param errorCb
     */
    void getUnreadNotificationCount(const std::optional<QDateTime>& seenAt, const
                                    UnreadCountSuccessCb& successCb, const ErrorCb& errorCb);

    void updateNotificationSeen(const QDateTime& dateTime,
                                const SuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief listNotifications
     * @param limit min=1, max=100, default=50
     * @param cursor
     * @param seenAt
     * @param successCb
     * @param errorCb
     * @param updateSeen update the seen timestamp to the time of sending this request
     */
    void listNotifications(std::optional<int> limit, const std::optional<QString>& cursor,
                           const std::optional<QDateTime>& seenAt,
                           const NotificationsSuccessCb& successCb, const ErrorCb& errorCb,
                           bool updateSeen = false);

    // com.atproto.repo

    /**
     * @brief uploadBlob
     * @param blob encoded image
     * @param mimeType mime-type of the image
     * @param successCb
     * @param errorCb
     */
    void uploadBlob(const QByteArray& blob, const QString& mimeType,
                    const UploadBlobSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getRecord
     * @param repo
     * @param collection
     * @param rkey
     * @param cid
     * @param successCb
     * @param errorCb
     */
    void getRecord(const QString& repo, const QString& collection,
                   const QString& rkey, const std::optional<QString>& cid,
                   const GetRecordSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief createRecord
     * @param repo
     * @param collection
     * @param record
     * @param successCb
     * @param errorCb
     */
    void createRecord(const QString& repo, const QString& collection, const QJsonObject& record,
                      const CreateRecordSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief deleteRecord
     * @param repo
     * @param collection
     * @param rkey
     * @param successCb
     * @param errorCb
     */
    void deleteRecord(const QString& repo, const QString& collection, const QString& rkey,
                      const SuccessCb& successCb, const ErrorCb& errorCb);

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
