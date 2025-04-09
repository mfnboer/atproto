// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "user_preferences.h"
#include "xjson.h"
#include "xrpc_client.h"
#include "lexicon/app_bsky_actor.h"
#include "lexicon/app_bsky_feed.h"
#include "lexicon/app_bsky_graph.h"
#include "lexicon/app_bsky_labeler.h"
#include "lexicon/app_bsky_notification.h"
#include "lexicon/app_bsky_unspecced.h"
#include "lexicon/app_bsky_video.h"
#include "lexicon/chat_bsky_convo.h"
#include "lexicon/com_atproto_moderation.h"
#include "lexicon/com_atproto_repo.h"
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
    using GetProfileSuccessCb = std::function<void(AppBskyActor::ProfileViewDetailed::SharedPtr)>;
    using GetProfilesSuccessCb = std::function<void(AppBskyActor::ProfileViewDetailedList)>;
    using GetAuthorFeedSuccessCb = std::function<void(AppBskyFeed::OutputFeed::SharedPtr)>;
    using GetActorLikesSuccessCb = std::function<void(AppBskyFeed::OutputFeed::SharedPtr)>;
    using GetTimelineSuccessCb = std::function<void(AppBskyFeed::OutputFeed::SharedPtr)>;
    using GetFeedSuccessCb = std::function<void(AppBskyFeed::OutputFeed::SharedPtr)>;
    using GetFeedGeneratorSuccessCb = std::function<void(AppBskyFeed::GetFeedGeneratorOutput::SharedPtr)>;
    using GetFeedGeneratorsSuccessCb = std::function<void(AppBskyFeed::GetFeedGeneratorsOutput::SharedPtr)>;
    using GetActorFeedsSuccessCb = std::function<void(AppBskyFeed::GetActorFeedsOutput::SharedPtr)>;
    using GetPostThreadSuccessCb = std::function<void(AppBskyFeed::PostThread::SharedPtr)>;
    using GetPostsSuccessCb = std::function<void(AppBskyFeed::PostViewList)>;
    using GetQuotesSuccessCb = std::function<void(AppBskyFeed::GetQuotesOutput::SharedPtr)>;
    using GetLikesSuccessCb = std::function<void(AppBskyFeed::GetLikesOutput::SharedPtr)>;
    using GetRepostedBySuccessCb = std::function<void(AppBskyFeed::GetRepostedByOutput::SharedPtr)>;
    using GetFollowsSuccessCb = std::function<void(AppBskyGraph::GetFollowsOutput::SharedPtr)>;
    using GetFollowersSuccessCb = std::function<void(AppBskyGraph::GetFollowersOutput::SharedPtr)>;
    using GetBlocksSuccessCb = std::function<void(AppBskyGraph::GetBlocksOutput::SharedPtr)>;
    using GetMutesSuccessCb = std::function<void(AppBskyGraph::GetMutesOutput::SharedPtr)>;
    using GetListSuccessCb = std::function<void(AppBskyGraph::GetListOutput::SharedPtr)>;
    using GetListsSuccessCb = std::function<void(AppBskyGraph::GetListsOutput::SharedPtr)>;
    using GetStarterPackSuccessCb = std::function<void(AppBskyGraph::StarterPackView::SharedPtr)>;
    using GetStarterPacksSuccessCb = std::function<void(AppBskyGraph::GetStarterPacksOutput::SharedPtr)>;
    using GetAccountInviteCodesSuccessCb = std::function<void(ComATProtoServer::GetAccountInviteCodesOutput::SharedPtr)>;
    using GetServiceAuthSuccessCb = std::function<void(ComATProtoServer::GetServiceAuthOutput::SharedPtr)>;
    using UploadBlobSuccessCb = std::function<void(Blob::SharedPtr)>;
    using GetBlobSuccessCb = std::function<void(const QByteArray& bytes, const QString& contentType)>;
    using GetRecordSuccessCb = std::function<void(ComATProtoRepo::Record::SharedPtr)>;
    using ListRecordsSuccessCb = std::function<void(ComATProtoRepo::ListRecordsOutput::SharedPtr)>;
    using CreateRecordSuccessCb = std::function<void(ComATProtoRepo::StrongRef::SharedPtr)>;
    using PutRecordSuccessCb = std::function<void(ComATProtoRepo::StrongRef::SharedPtr)>;
    using UnreadCountSuccessCb = std::function<void(int)>;
    using NotificationsSuccessCb = std::function<void(AppBskyNotification::ListNotificationsOutput::SharedPtr)>;
    using UserPrefsSuccessCb = std::function<void(UserPreferences)>;
    using SearchActorsSuccessCb = std::function<void(AppBskyActor::SearchActorsOutput::SharedPtr)>;
    using SearchActorsTypeaheadSuccessCb = std::function<void(AppBskyActor::SearchActorsTypeaheadOutput::SharedPtr)>;
    using SearchPostsSuccessCb = std::function<void(AppBskyFeed::SearchPostsOutput::SharedPtr)>;
    using GetPopularFeedGeneratorsSuccessCb = std::function<void(AppBskyUnspecced::GetPopularFeedGeneratorsOutput::SharedPtr)>;
    using GetTrendingTopicsSuccessCb = std::function<void(AppBskyUnspecced::GetTrendingTopicsOutput::SharedPtr)>;
    using GetSuggestionsSuccessCb = std::function<void(AppBskyActor::GetSuggestionsOutput::SharedPtr)>;
    using GetSuggestedFollowsSuccessCb = std::function<void(AppBskyActor::GetSuggestedFollowsByActor::SharedPtr)>;
    using GetServicesSuccessCb = std::function<void(AppBskyLabeler::GetServicesOutput::SharedPtr)>;
    using VideoJobStatusOutputCb = std::function<void(AppBskyVideo::JobStatusOutput::SharedPtr)>;
    using VideoUploadOutputCb = std::function<void(AppBskyVideo::JobStatus::SharedPtr)>;
    using GetVideoUploadLimitsCb = std::function<void(AppBskyVideo::GetUploadLimitsOutput::SharedPtr)>;

    using DeleteMessageSuccessCb = std::function<void(ChatBskyConvo::DeletedMessageView::SharedPtr)>;
    using AcceptConvoSuccessCb = std::function<void(ChatBskyConvo::AcceptConvoOutput::SharedPtr)>;
    using ConvoSuccessCb = std::function<void(ChatBskyConvo::ConvoOutput::SharedPtr)>;
    using ConvoAvailabilitySuccessCb = std::function<void(ChatBskyConvo::ConvoAvailabilityOuput::SharedPtr)>;
    using ConvoListSuccessCb = std::function<void(ChatBskyConvo::ConvoListOutput::SharedPtr)>;
    using ConvoLogSuccessCb = std::function<void(ChatBskyConvo::LogOutput::SharedPtr)>;
    using GetMessagesSuccessCb = std::function<void(ChatBskyConvo::GetMessagesOutput::SharedPtr)>;
    using LeaveConvoSuccessCb = std::function<void(ChatBskyConvo::LeaveConvoOutput::SharedPtr)>;
    using MessageSuccessCb = std::function<void(ChatBskyConvo::MessageView::SharedPtr)>;
    using UpdateAllReadSuccessCb = std::function<void(ChatBskyConvo::UpdateAllReadOutput::SharedPtr)>;
    using ReactionSuccessCb = std::function<void(ChatBskyConvo::MessageOutput::SharedPtr)>;

    using ErrorCb = std::function<void(const QString& error, const QString& message)>;

    using Ptr = std::unique_ptr<Client>;

    static constexpr int MAX_LABELERS = 20;
    static constexpr int MAX_URIS_GET_POSTS = 25;
    static constexpr int MAX_URIS_GET_STARTER_PACKS = 25;
    static constexpr int MAX_IDS_GET_PROFILES = 25;
    static constexpr int MAX_CONVO_MEMBERS = 10;

    static bool isListNotFoundError(const QString& error);

    explicit Client(Xrpc::Client::Ptr&& xrpc);

    const QString& getPDS() const { return mXrpc->getPDS(); }
    const ComATProtoServer::Session* getSession() const { return mSession.get(); }
    void setSession(ComATProtoServer::Session::SharedPtr session) { mSession = std::move(session); }
    void clearSession() { mSession = nullptr; }
    void updateTokens(const QString& accessJwt, const QString& refreshJwt);

    bool setLabelerDids(const std::unordered_set<QString>& dids);
    bool addLabelerDid(const QString& did);
    void removeLabelerDid(const QString& did);

    // com.atproto.server
    /**
     * @brief createSession
     * @param user user handle or did
     * @param pwd password
     * @param authFactorToken 2FA token
     * @param successCb
     * @param errorCb
     *
     * PDS will be resolved from the user
     */
    void createSession(const QString& user, const QString& pwd,
                       const std::optional<QString>& authFactorToken,
                       const SuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief deleteSession Delete the current session
     * @param successCb
     * @param errorCb
     */
    void deleteSession(const SuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief resumeSession Resume a previously created session
     * @param session
     * @param successCb
     * @param errorCb
     *
     * PDS will be resolved from the user
     */
    void resumeSession(const ComATProtoServer::Session& session,
                       const SuccessCb& successCb, const ErrorCb& errorCb);

    void refreshSession(const SuccessCb& successCb, const ErrorCb& errorCb);

    void getAccountInviteCodes(const GetAccountInviteCodesSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getServiceAuth
     * @param aud
     * @param expiry
     * @param lexiconMethod
     * @param successCb
     * @param errorCb
     */
    void getServiceAuth(const QString& aud, const std::optional<QDateTime>& expiry, const std::optional<QString>& lexiconMethod,
                        const GetServiceAuthSuccessCb& successCb, const ErrorCb& errorCb);

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

    /**
     * @brief searchActorsTypeahead
     * @param q search query prefix
     * @param limit min=1 max=100 default=10
     * @param successCb
     * @param errorCb
     */
    void searchActorsTypeahead(const QString& q, std::optional<int> limit,
                               const SearchActorsTypeaheadSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getSuggestions
     * @param limit min=1 max=100 default=50
     * @param cursor
     * @param acceptLanguages add HTTP Accept-Language if languages are set
     * @param successCb
     * @param errorCb
     */
    void getSuggestions(std::optional<int> limit, const std::optional<QString>& cursor,
                        const QStringList& acceptLanguages,
                        const GetSuggestionsSuccessCb& successCb, const ErrorCb& errorCb);

    // app.bsky.labeler
    /**
     * @brief getServices
     * @param dids
     * @param detailed
     * @param successCb
     * @param errorCb
     */
    void getServices(const std::vector<QString>& dids, bool detailed,
                     const GetServicesSuccessCb& successCb, const ErrorCb& errorCb);

    // app.bsky.feed
    /**
     * @brief getAuthorFeed
     * @param user user handle or did
     * @param limit min=1 max=100 default=50
     * @param cursor
     * @param filter "posts_with_replies", "posts_no_replies", "posts_with_media", "posts_and_author_threads", "posts_width_video" default="posts_with_replies"
     * @paran includePins default = false
     * @param successCb
     * @param errorCb
     */
    void getAuthorFeed(const QString& user, std::optional<int> limit, const std::optional<QString>& cursor,
                       const std::optional<QString> filter, std::optional<bool> includePins,
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
     * @param acceptLanguages add HTTP Accept-Language if languages are set
     * @param successCb
     * @param errorCb
     */
    void getFeed(const QString& feed, std::optional<int> limit, const std::optional<QString>& cursor,
                 const QStringList& acceptLanguages,
                 const GetFeedSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getListFeed
     * @param list list at-uri
     * @param limit min=1 max=100 default=50
     * @param cursor
     * @param acceptLanguages add HTTP Accept-Language if languages are set
     * @param successCb
     * @param errorCb
     */
    void getListFeed(const QString& list, std::optional<int> limit, const std::optional<QString>& cursor,
                     const QStringList& acceptLanguages,
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
     * @brief getActorFeeds Get a list of feeds created by the actor.
     * @param user user handle or did
     * @param limit min=1 max=100 default=50
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getActorFeeds(const QString& user, std::optional<int> limit, const std::optional<QString>& cursor,
                       const GetActorFeedsSuccessCb& successCb, const ErrorCb& errorCb);

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
     * @brief getQuotes
     * @param uri get quotes for this post
     * @param cid
     * @param limit min=1 max=100 default=50
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getQuotes(const QString& uri, const std::optional<QString>& cid, std::optional<int> limit,
                   const std::optional<QString>& cursor, const GetQuotesSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief searchPosts
     * @param q search query
     * @param limit min=1 max=100 default=25
     * @param cursor
     * @param sort "top", "latest" default="latest"
     * @param author at-identifier, post from this author
     * @param mentions at-identifier, posts mentioning this user
     * @param since posts from this timestamp UTC (inclusive)
     * @param until posts until this timestamp UTC (exclusive)
     * @param lang posts in this language, 2-letter language code
     * @param successCb
     * @param errorCb
     */
    void searchPosts(const QString& q, std::optional<int> limit, const std::optional<QString>& cursor,
                     const std::optional<QString>& sort, const std::optional<QString>& author,
                     const std::optional<QString>& mentions, const std::optional<QDateTime>& since,
                     const std::optional<QDateTime>& until, const std::optional<QString>& lang,
                     const SearchPostsSuccessCb& successCb, const ErrorCb& errorCb);

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

    /**
     * @brief sendInteractions
     * @param interactions
     * @param feedDid DID of the feed generator to which the feedback should be sent
     * @param successCb
     * @param errorCb
     */
    void sendInteractions(const AppBskyFeed::InteractionList& interactions, const QString& feedDid,
                          const SuccessCb& successCb, const ErrorCb& errorCb);

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
    void getKnownFollowers(const QString& actor, std::optional<int> limit, const std::optional<QString>& cursor,
                           const GetFollowersSuccessCb& successCb, const ErrorCb& errorCb);

    void getBlocks(std::optional<int> limit, const std::optional<QString>& cursor,
                   const GetBlocksSuccessCb& successCb, const ErrorCb& errorCb);
    void getMutes(std::optional<int> limit, const std::optional<QString>& cursor,
                  const GetMutesSuccessCb& successCb, const ErrorCb& errorCb);

    void muteActor(const QString& actor, const SuccessCb& successCb, const ErrorCb& errorCb);
    void unmuteActor(const QString& actor, const SuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief muteThread
     * @param root at-uri
     * @param successCb
     * @param errorCb
     */
    void muteThread(const QString& root, const SuccessCb& successCb, const ErrorCb& errorCb);
    void unmuteThread(const QString& root, const SuccessCb& successCb, const ErrorCb& errorCb);

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

    /**
     * @brief getActorStarterPacks
     * @param actor min=1 default=50 max=100
     * @param limit
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getActorStarterPacks(const QString& actor, std::optional<int> limit, const std::optional<QString>& cursor,
                              const GetStarterPacksSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getStarterPacks
     * @param uris max 25 at-uri's
     * @param successCb
     * @param errorCb
     */
    void getStarterPacks(const std::vector<QString>& uris,
                         const GetStarterPacksSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getStarterPack
     * @param starterPack at-uri of starter pack record
     * @param successCb
     * @param errorCb
     */
    void getStarterPack(const QString& starterPack, const GetStarterPackSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getSuggestedFollows
     * @param user handle or did
     * @param successCb
     * @param errorCb
     */
    void getSuggestedFollows(const QString& user, const QStringList& acceptLanguages,
                             const GetSuggestedFollowsSuccessCb& successCb, const ErrorCb& errorCb);

    // app.bsky.notification

    /**
     * @brief getUnreadCount Get the number of unread notifications since last time.
     * @param seenAt Last timestamp UTC the notifications were seen
     * @param priority Filter priority notifications (to override preference)
     * @param successCb
     * @param errorCb
     */
    void getUnreadNotificationCount(const std::optional<QDateTime>& seenAt, std::optional<bool> priority,
                                    const UnreadCountSuccessCb& successCb, const ErrorCb& errorCb);

    void updateNotificationSeen(const QDateTime& dateTime,
                                const SuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief listNotifications
     * @param limit min=1, max=100, default=50
     * @param cursor
     * @param seenAt
     * @param priority Filter priority notifications (to override preference)
     * @param successCb
     * @param errorCb
     * @param updateSeen update the seen timestamp to the time of sending this request
     */
    void listNotifications(std::optional<int> limit, const std::optional<QString>& cursor,
                           const std::optional<QDateTime>& seenAt, std::optional<bool> priority,
                           const std::vector<AppBskyNotification::NotificationReason> reasons,
                           const NotificationsSuccessCb& successCb, const ErrorCb& errorCb,
                           bool updateSeen = false);

    void putNotificationPreferences(bool priority,
                                    const SuccessCb& successCb, const ErrorCb& errorCb);

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

    // app.bsky.video

    /**
     * @brief getVideoJobStatus
     * @param jobId
     * @param successCb
     * @param errorCb
     */
    void getVideoJobStatus(const QString& jobId, const VideoJobStatusOutputCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getVideoUploadLimits
     * @param successCb
     * @param errorCb
     */
    void getVideoUploadLimits(const GetVideoUploadLimitsCb& successCb, const ErrorCb& errorCb);
    void getVideoUploadLimits(const QString& serviceAuthToken, const GetVideoUploadLimitsCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief uploadVideo
     * @param blob mp4 encoded video
     * @param successCb
     * @param errorCb
     */
    void uploadVideo(QFile* blob, const VideoUploadOutputCb& successCb, const ErrorCb& errorCb);
    void uploadVideo(QFile* blob, const QString& serviceAuthToken, const VideoUploadOutputCb& successCb, const ErrorCb& errorCb);

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
     * @brief listRecords
     * @param repo
     * @param collection
     * @param limit min=1, max=100, default=50
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void listRecords(const QString& repo, const QString& collection,
                     std::optional<int> limit, const std::optional<QString>& cursor,
                     const ListRecordsSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief createRecord
     * @param repo
     * @param collection
     * @param rkey if empty, then an rkey will be generated by the server
     * @param record
     * @param validate Can be set to 'false' to skip Lexicon schema validation of record data.
     * @param successCb
     * @param errorCb
     */
    void createRecord(const QString& repo, const QString& collection, const QString& rkey,
                      const QJsonObject& record, bool validate,
                      const CreateRecordSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief putRecord create or update a record
     * @param repo
     * @param collection
     * @param rkey
     * @param record
     * @param validate Can be set to 'false' to skip Lexicon schema validation of record data.
     * @param successCb
     * @param errorCb
     */
    void putRecord(const QString& repo, const QString& collection, const QString& rkey,
                   const QJsonObject& record, bool validate,
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

    /**
     * @brief applyWrites
     * @param repo
     * @param writes
     * @param validate
     * @param successCb
     * @param errorCb
     */
    void applyWrites(const QString& repo, const ComATProtoRepo::ApplyWritesList& writes, bool validate,
                     const SuccessCb& successCb, const ErrorCb& errorCb);

    // com.atproto.sync

    /**
     * @brief getBlob
     * @param did
     * @param cid
     * @param successCb
     * @param errorCb
     *
     * PDS will be resolved from the did
     */
    void getBlob(const QString& did, const QString& cid,
                 const GetBlobSuccessCb& successCb, const ErrorCb& errorCb);

    // com.atproto.moderation

    /**
     * @brief reportAuthor
     * @param did
     * @param reasonType
     * @param reason
     * @param labelerDid if set, then send the report to this labeler
     * @param successCb
     * @param errorCb
     */
    void reportAuthor(const QString& did, ComATProtoModeration::ReasonType reasonType,
                      const QString& reason, const std::optional<QString>& labelerDid,
                      const SuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief reportPostOrFeed
     * @param uri post or feed at-uri
     * @param cid
     * @param reasonType
     * @param reason
     * @param labelerDid if set, then send the report to this labeler
     * @param successCb
     * @param errorCb
     */
    void reportPostOrFeed(const QString& uri, const QString& cid,
                          ComATProtoModeration::ReasonType reasonType,
                          const QString& reason, const std::optional<QString>& labelerDid,
                          const SuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief reportDirectMessage
     * @param did
     * @param convoId
     * @param messageId
     * @param reasonType
     * @param reason
     * @param successCb
     * @param errorCb
     */
    void reportDirectMessage(const QString& did, const QString& convoId, const QString& messageId,
                             ComATProtoModeration::ReasonType reasonType, const QString& reason,
                             const SuccessCb& successCb, const ErrorCb& errorCb);

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

    /**
     * @brief getTrendingTopics
     * @param viewer DID of the account making the request (not included for public/unauthenticated queries). Used to boost followed accounts in ranking.
     * @param limit min=1 default=10 max=25
     * @param successCb
     * @param errorCb
     */
    void getTrendingTopics(const std::optional<QString>& viewer, std::optional<int> limit,
                           const GetTrendingTopicsSuccessCb& successCb, const ErrorCb& errorCb);

    // chat.bsky.convo

    /**
     * @brief acceptConvo
     * @param convoId
     * @param successCb
     * @param errorCb
     */
    void acceptConvo(const QString& convoId,
                     const AcceptConvoSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief deleteMessageForSelf
     * @param convoId
     * @param messageId
     * @param successCb
     * @param errorCb
     */
    void deleteMessageForSelf(const QString& convoId, const QString& messageId,
                              const DeleteMessageSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getConvo
     * @param convoId
     * @param successCb
     * @param errorCb
     */
    void getConvo(const QString& convoId,
                  const ConvoSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getConvoForMembers
     * @param members list of DID's (min=1 max=10)
     * @param successCb
     * @param errorCb
     */
    void getConvoForMembers(const std::vector<QString>& members,
                            const ConvoSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getConvoAvailability Get whether the requester and the other members can chat. If an existing convo is found for these members, it is returned.
     * @param members list of DID's (min=1 max=10)
     * @param successCb
     * @param errorCb
     */
    void getConvoAvailability(const std::vector<QString>& members,
                              const ConvoAvailabilitySuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getConvoLog
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getConvoLog(const std::optional<QString>& cursor,
                     const ConvoLogSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief getMessages
     * @param convoId
     * @param limit min=1, max=100, default=50
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void getMessages(const QString& convoId, std::optional<int> limit,
                     const std::optional<QString>& cursor,
                     const GetMessagesSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief leaveConvo
     * @param convoId
     * @param successCb
     * @param errorCb
     */
    void leaveConvo(const QString& convoId,
                    const LeaveConvoSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief listConvos
     * @param limit min=1, max=100, default=50
     * @param onlyUnread
     * @param status
     * @param cursor
     * @param successCb
     * @param errorCb
     */
    void listConvos(std::optional<int> limit, bool onlyUnread,
                    std::optional<ChatBskyConvo::ConvoStatus> status,
                    const std::optional<QString>& cursor,
                    const ConvoListSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief muteConvo
     * @param convoId
     * @param successCb
     * @param errorCb
     */
    void muteConvo(const QString& convoId,
                   const ConvoSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief sendMessage
     * @param convoId
     * @param message
     * @param successCb
     * @param errorCn
     */
    void sendMessage(const QString& convoId, const ChatBskyConvo::MessageInput& message,
                     const MessageSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief unmuteConvo
     * @param convoId
     * @param successCb
     * @param errorCb
     */
    void unmuteConvo(const QString& convoId,
                     const ConvoSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief updateRead
     * @param convoId
     * @param messageId
     * @param successCb
     * @param errorCb
     */
    void updateRead(const QString& convoId, const std::optional<QString>& messageId,
                    const ConvoSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief updateAllRead
     * @param status
     * @param successCb
     * @param errorCb
     */
    void updateAllRead(std::optional<ChatBskyConvo::ConvoStatus> status,
                       const UpdateAllReadSuccessCb& successCb, const ErrorCb& errorCb);

    /**
     * @brief addReaction
     * @param convoId
     * @param messageId
     * @param value minLength=1 maxLength=32 minGraphemes=1 maxGraphemes=1
     * @param successCb
     * @param errorCb
     */
    void addReaction(const QString& convoId, const QString& messageId, const QString& value,
                     const ReactionSuccessCb& successCb, const ErrorCb& errorCb);
    void removeReaction(const QString& convoId, const QString& messageId, const QString& value,
                        const ReactionSuccessCb& successCb, const ErrorCb& errorCb);

private:
    const QString& authToken() const;
    const QString& refreshToken() const;

    // Create XRPC error callback from ATProto client callback
    Xrpc::Client::ErrorCb failure(const ErrorCb& cb);

    void invalidJsonError(InvalidJsonException& e, const ErrorCb& cb);
    void requestFailed(const QString& err, const QJsonDocument& json, const ErrorCb& errorCb);

    void setAcceptLabelersHeaderValue();
    void addAcceptLabelersHeader(Xrpc::Client::Params& httpHeaders) const;
    void addAcceptLanguageHeader(Xrpc::Client::Params& httpHeaders, const QStringList& languages) const;
    void addAtprotoProxyHeader(Xrpc::Client::Params& httpHeaders, const QString& did, const QString& serviceKey) const;

    void createSessionContinue(const QString& user, const QString& pwd,
                               const std::optional<QString>& authFactorToken,
                               const SuccessCb& successCb, const ErrorCb& errorCb);
    void resumeSessionContinue(const ComATProtoServer::Session& session,
                               const SuccessCb& successCb, const ErrorCb& errorCb);
    void getBlobContinue(const QString& did, const QString& cid,
                         const GetBlobSuccessCb& successCb, const ErrorCb& errorCb);

    Xrpc::Client::Ptr mXrpc;
    ComATProtoServer::Session::SharedPtr mSession;
    std::unordered_set<QString> mLabelerDids;
    QString mAcceptLabelersHeaderValue;
};

}
