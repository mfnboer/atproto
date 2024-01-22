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
#include "lexicon/app_bsky_unspecced.h"
#include "lexicon/com_atproto_moderation.h"
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
    using GetProfilesSuccessCb = std::function<void(AppBskyActor::ProfileViewDetailedList)>;
    using GetAuthorFeedSuccessCb = std::function<void(AppBskyFeed::OutputFeed::Ptr)>;
    using GetActorLikesSuccessCb = std::function<void(AppBskyFeed::OutputFeed::Ptr)>;
    using GetTimelineSuccessCb = std::function<void(AppBskyFeed::OutputFeed::Ptr)>;
    using GetFeedSuccessCb = std::function<void(AppBskyFeed::OutputFeed::Ptr)>;
    using GetFeedGeneratorSuccessCb = std::function<void(AppBskyFeed::GetFeedGeneratorOutput::Ptr)>;
    using GetFeedGeneratorsSuccessCb = std::function<void(AppBskyFeed::GetFeedGeneratorsOutput::Ptr)>;
    using GetPostThreadSuccessCb = std::function<void(AppBskyFeed::PostThread::Ptr)>;
    using GetPostsSuccessCb = std::function<void(AppBskyFeed::PostViewList)>;
    using GetLikesSuccessCb = std::function<void(AppBskyFeed::GetLikesOutput::Ptr)>;
    using GetRepostedBySuccessCb = std::function<void(AppBskyFeed::GetRepostedByOutput::Ptr)>;
    using GetFollowsSuccessCb = std::function<void(AppBskyGraph::GetFollowsOutput::Ptr)>;
    using GetFollowersSuccessCb = std::function<void(AppBskyGraph::GetFollowersOutput::Ptr)>;
    using GetBlocksSuccessCb = std::function<void(AppBskyGraph::GetBlocksOutput::Ptr)>;
    using GetMutesSuccessCb = std::function<void(AppBskyGraph::GetMutesOutput::Ptr)>;
    using GetListSuccessCb = std::function<void(AppBskyGraph::GetListOutput::Ptr)>;
    using GetListsSuccessCb = std::function<void(AppBskyGraph::GetListsOutput::Ptr)>;
    using GetAccountInviteCodesSuccessCb = std::function<void(ComATProtoServer::GetAccountInviteCodesOutput::Ptr)>;
    using UploadBlobSuccessCb = std::function<void(Blob::Ptr)>;
    using GetRecordSuccessCb = std::function<void(ComATProtoRepo::Record::Ptr)>;
    using CreateRecordSuccessCb = std::function<void(ComATProtoRepo::StrongRef::Ptr)>;
    using PutRecordSuccessCb = std::function<void(ComATProtoRepo::StrongRef::Ptr)>;
    using UnreadCountSuccessCb = std::function<void(int)>;
    using NotificationsSuccessCb = std::function<void(AppBskyNotification::ListNotificationsOutput::Ptr)>;
    using UserPrefsSuccessCb = std::function<void(UserPreferences)>;
    using SearchActorsSuccessCb = std::function<void(AppBskyActor::SearchActorsOutput::Ptr)>;
    using SearchActorsTypeaheadSuccessCb = std::function<void(AppBskyActor::SearchActorsTypeaheadOutput::Ptr)>;
    using SearchPostsSuccessCb = std::function<void(AppBskyFeed::SearchPostsOutput::Ptr)>;
    using LegacySearchPostsSuccessCb = std::function<void(AppBskyFeed::LegacySearchPostsOutput::Ptr)>;
    using LegacySearchActorsSuccessCb = std::function<void(AppBskyActor::LegacySearchActorsOutput::Ptr)>;
    using GetPopularFeedGeneratorsSuccessCb = std::function<void(AppBskyUnspecced::GetPopularFeedGeneratorsOutput::Ptr)>;
    using ErrorCb = std::function<void(const QString& error, const QString& message)>;

    static constexpr int MAX_URIS_GET_POSTS = 25;
    static constexpr int MAX_IDS_GET_PROFILES = 25;

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

    void refreshSession(const SuccessCb& successCb, const ErrorCb& errorCb);

    void getAccountInviteCodes(const GetAccountInviteCodesSuccessCb& successCb, const ErrorCb& errorCb);

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

    /**
     * @brief getProfiles
     * @param users list of handles or dids (max 25)
     * @param successCb
     * @param errorCb
     */
    void getProfiles(const std::vector<QString>& users, const GetProfilesSuccessCb& successCb, const ErrorCb& errorCb);

    void getPreferences(const UserPrefsSuccessCb& successCb, const ErrorCb& errorCb);
    void putPreferences(const UserPreferences& userPrefs,
                        const SuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief searchActors
     * @param q search query
     * @param limit min=1 max=100 default=25
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void searchActors(const QString& q, std::optional<int> limit, const std::optional<QString>& cursor,
                      const SearchActorsSuccessCb& successCb, const ErrorCb& errorCb);

    // TODO: remove legacy search
    // https://search.bsky.social/search/posts?q=
    // https://github.com/bluesky-social/social-app/blob/7ebf1ed3710081f27f90eaae125c7315798d56e5/src/lib/api/search.ts#L41
    void legacySearchActors(const QString& q,
                             const LegacySearchActorsSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief searchActorsTypeahead
     * @param q search query prefix
     * @param limit min=1 max=100 default=10
     * @param successCb
     * @param errorCb
     */
    void searchActorsTypeahead(const QString& q, std::optional<int> limit,
                               const SearchActorsTypeaheadSuccessCb& successCb, const ErrorCb& errorCb);

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
     * @brief getActorLikes Get a list of posts liked by an actor.
     * @param user user handle or did
     * @param limit min=1 max=100 default=50
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getActorLikes(const QString& user, std::optional<int> limit, const std::optional<QString>& cursor,
                       const GetActorLikesSuccessCb& successCb, const ErrorCb& errorCb);

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
     * @brief getFeed
     * @param feed feed at-uri
     * @param limit min=1 max=100 default=50
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getFeed(const QString& feed, std::optional<int> limit, const std::optional<QString>& cursor,
                 const GetFeedSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getListFeed
     * @param list list at-uri
     * @param limit min=1 max=100 default=50
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getListFeed(const QString& list, std::optional<int> limit, const std::optional<QString>& cursor,
                     const GetFeedSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getFeedGenerator
     * @param feed feed at-uri
     * @param successCb
     * @param errorCb
     */
    void getFeedGenerator(const QString& feed,
                          const GetFeedGeneratorSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getFeedGenerators
     * @param feeds list of feed at-uri's
     * @param successCb
     * @param errorCb
     */
    void getFeedGenerators(const std::vector<QString>& feeds,
                           const GetFeedGeneratorsSuccessCb& successCb, const ErrorCb& errorCb);

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
     * @brief searchPosts
     * @param q search query
     * @param limit min=1 max=100 default=25
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void searchPosts(const QString& q, std::optional<int> limit, const std::optional<QString>& cursor,
                     const SearchPostsSuccessCb& successCb, const ErrorCb& errorCb);

    // TODO: remove legacy search
    // https://search.bsky.social/search/posts?q=
    // https://github.com/bluesky-social/social-app/blob/7ebf1ed3710081f27f90eaae125c7315798d56e5/src/lib/api/search.ts#L41
    void legacySearchPosts(const QString& q,
                           const LegacySearchPostsSuccessCb& successCb, const ErrorCb& errorCb);

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
     * @param actor handle or did
     * @param limit min=1 default=50 max=100
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getFollows(const QString& actor, std::optional<int> limit, const std::optional<QString>& cursor,
                    const GetFollowsSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getFollowers
     * @param actor handle or did
     * @param limit min=1 default=50 max=100
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getFollowers(const QString& actor, std::optional<int> limit, const std::optional<QString>& cursor,
                      const GetFollowersSuccessCb& successCb, const ErrorCb& errorCb);

    void getBlocks(std::optional<int> limit, const std::optional<QString>& cursor,
                   const GetBlocksSuccessCb& successCb, const ErrorCb& errorCb);
    void getMutes(std::optional<int> limit, const std::optional<QString>& cursor,
                  const GetMutesSuccessCb& successCb, const ErrorCb& errorCb);

    void muteActor(const QString& actor, const SuccessCb& successCb, const ErrorCb& errorCb);
    void unmuteActor(const QString& actor, const SuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getList
     * @param listUri at-uri of list
     * @param limit min=1 default=50 max=100
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getList(const QString& listUri, std::optional<int> limit, const std::optional<QString>& cursor,
                 const GetListSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getLists Get a list of lists that belong to an actor.
     * @param actor handle or did
     * @param limit min=1 default=50 max=100
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getLists(const QString& actor, std::optional<int> limit, const std::optional<QString>& cursor,
                 const GetListsSuccessCb& successCb, const ErrorCb& errorCb);

    void muteActorList(const QString& listUri, const SuccessCb& successCb, const ErrorCb& errorCb);
    void unmuteActorList(const QString& listUri, const SuccessCb& successCb, const ErrorCb& errorCb);


    /**
     * @brief getListBlocks Get lists that the actor is blocking.
     * @param limit min=1 default=50 max=100
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getListBlocks(std::optional<int> limit, const std::optional<QString>& cursor,
                       const GetListsSuccessCb& successCb, const ErrorCb& errorCb);
    void getListMutes(std::optional<int> limit, const std::optional<QString>& cursor,
                      const GetListsSuccessCb& successCb, const ErrorCb& errorCb);

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

    /**
     * @brief registerPushNotifications
     * @param serviceDid
     * @param token Firebase messaging token
     * @param platform ios, android or web
     * @param appid
     * @param successCb
     * @param errorCb
     */
    void registerPushNotifications(const QString& serviceDid, const QString& token,
                                   const QString& platform, const QString& appId,
                                   const SuccessCb& successCb, const ErrorCb& errorCb);

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
     * @param rkey if empty, then an rkey will be generated by the server
     * @param record
     * @param successCb
     * @param errorCb
     */
    void createRecord(const QString& repo, const QString& collection, const QString& rkey, const QJsonObject& record,
                      const CreateRecordSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief putRecord create or update a record
     * @param repo
     * @param collection
     * @param rkey
     * @param record
     * @param successCb
     * @param errorCb
     */
    void putRecord(const QString& repo, const QString& collection, const QString& rkey, const QJsonObject& record,
                   const PutRecordSuccessCb& successCb, const ErrorCb& errorCb);

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

    // com.atproto.moderation

    /**
     * @brief reportAuthor
     * @param did
     * @param reasonType
     * @param reason
     * @param successCb
     * @param errorCb
     */
    void reportAuthor(const QString& did, ComATProtoModeration::ReasonType reasonType,
                      const QString& reason, const SuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief reportPostOrFeed
     * @param uri post or feed at-uri
     * @param cid
     * @param reasonType
     * @param reason
     * @param successCb
     * @param errorCb
     */
    void reportPostOrFeed(const QString& uri, const QString& cid, ComATProtoModeration::ReasonType reasonType,
                    const QString& reason, const SuccessCb& successCb, const ErrorCb& errorCb);

    // app.bsky.unspecced

    /**
     * @brief getPopularFeedGenerators
     * @param query
     * @param limit min=1 default=50 max=100
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getPopularFeedGenerators(const std::optional<QString>& q, std::optional<int> limit,
                                  const std::optional<QString>& cursor,
                                  const GetPopularFeedGeneratorsSuccessCb& successCb, const ErrorCb& errorCb);

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
