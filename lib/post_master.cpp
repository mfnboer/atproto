// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "post_master.h"
#include <QTimer>

namespace ATProto {

PostMaster::PostMaster(Client& client) :
    Presence(),
    mClient(client),
    mRichTextMaster(client)
{
}

void PostMaster::post(const ATProto::AppBskyFeed::Record::Post& post,
                      const PostSuccessCb& successCb, const ErrorCb& errorCb)
{
    QJsonObject postJson;

    try {
        postJson = post.toJson();
    } catch (InvalidContent& e) {
        if (errorCb)
            QTimer::singleShot(0, &mPresence, [errorCb, e]{ errorCb("InvalidContent", "Invalid content: " + e.msg()); });

        return;
    }

    qDebug() << "Posting:" << postJson;
    const QString& repo = mClient.getSession()->mDid;
    const QString collection = postJson["$type"].toString();

    mClient.createRecord(repo, collection, {}, postJson, true,
        [successCb](auto strongRef){
            if (successCb)
                successCb(strongRef->mUri, strongRef->mCid);
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

void PostMaster::addThreadgate(const QString& uri, bool allowMention, bool allowFollower, bool allowFollowing, const QStringList& allowLists,
                               bool allowNobody, const QStringList& hiddenReplies,
                               const ThreadgateSuccessCb& successCb, const ErrorCb& errorCb)
{
    const auto atUri = ATUri::createAtUri(uri, mPresence, errorCb);
    if (!atUri.isValid())
        return;

    auto threadgate = createThreadgate(uri, allowMention, allowFollower, allowFollowing, allowLists, allowNobody, hiddenReplies);
    QJsonObject threadgateJson = threadgate->toJson();
    qDebug() << "Add threadgate:" << threadgateJson;
    const QString& repo = mClient.getSession()->mDid;
    const QString collection = threadgateJson["$type"].toString();

    mClient.putRecord(repo, collection, atUri.getRkey(), threadgateJson, true,
        [successCb](auto strongRef){
            if (successCb)
                successCb(strongRef->mUri, strongRef->mCid);
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

void PostMaster::addPostgate(const QString& uri, bool disableEmbedding, const QStringList& detachedEmbeddingUris,
                             const PostgateSuccessCb& successCb, const ErrorCb& errorCb)
{
    const auto atUri = ATUri::createAtUri(uri, mPresence, errorCb);
    if (!atUri.isValid())
        return;

    auto postgate = createPostgate(uri, disableEmbedding, detachedEmbeddingUris);
    QJsonObject postgateJson = postgate->toJson();
    qDebug() << "Add postgate:" << postgateJson;
    const QString& repo = mClient.getSession()->mDid;
    const QString collection = postgateJson["$type"].toString();

    mClient.putRecord(repo, collection, atUri.getRkey(), postgateJson, true,
        [successCb](auto strongRef){
            if (successCb)
                successCb(strongRef->mUri, strongRef->mCid);
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

void PostMaster::detachEmbedding(const QString& uri, const QString& embeddingUri, const QString& embeddingCid,
                                 bool detach, const EmbeddingDetachedCb& successCb, const ErrorCb& errorCb)
{
    getPostgate(uri,
        [this, presence=getPresence(), uri, embeddingUri, embeddingCid, detach, successCb, errorCb](auto postgate){
            if (!presence)
                return;

            continueDetachEmbedding(uri, embeddingUri, embeddingCid, postgate->mDetachedEmbeddingUris, detach, successCb, errorCb);
        },
        [this, presence=getPresence(), uri, embeddingUri, embeddingCid, detach, successCb, errorCb](const QString& error, const QString& msg){
            if (!presence)
                return;

            if (ATProto::ATProtoErrorMsg::isRecordNotFound(error))
                continueDetachEmbedding(uri, embeddingUri, embeddingCid, {}, detach, successCb, errorCb);
            else if (errorCb)
                errorCb(error, msg);
        });
}

void PostMaster::continueDetachEmbedding(const QString& uri, const QString& embeddingUri, const QString& embeddingCid,
                             const std::vector<QString>& currentDetachedEmbeddingUris, bool detach,
                             const EmbeddingDetachedCb& successCb, const ErrorCb& errorCb)
{
    QStringList detachedEmbeddingUris(currentDetachedEmbeddingUris.begin(), currentDetachedEmbeddingUris.end());

    if (detach)
        detachedEmbeddingUris.push_back(embeddingUri);
    else
        detachedEmbeddingUris.removeOne(embeddingUri);

    addPostgate(uri, false, detachedEmbeddingUris,
        [embeddingUri, embeddingCid, detach, successCb](const QString&, const QString&){
            if (successCb)
                successCb(embeddingUri, embeddingCid, detach);
        },
        [errorCb](const QString& error, const QString& msg){
            if (errorCb)
                errorCb(error, msg);
        });
}

AppBskyFeed::Threadgate::SharedPtr PostMaster::createThreadgate(const QString& uri, bool allowMention, bool allowFollower,
        bool allowFollowing, const QStringList& allowLists, bool allowNobody, const QStringList& hiddenReplies)
{
    auto threadgate = std::make_shared<AppBskyFeed::Threadgate>();
    threadgate->mPost = uri;
    threadgate->mRules.mAllowNobody = allowNobody;
    threadgate->mRules.mAllowMention = allowMention;
    threadgate->mRules.mAllowFollower = allowFollower;
    threadgate->mRules.mAllowFollowing = allowFollowing;
    threadgate->mHiddenReplies.insert(hiddenReplies.begin(), hiddenReplies.end());
    threadgate->mCreatedAt = QDateTime::currentDateTimeUtc();

    for (const auto& listUri : allowLists)
    {
        auto listRule = std::make_shared<AppBskyFeed::ThreadgateListRule>();
        listRule->mList = listUri;
        threadgate->mRules.mAllowList.push_back(std::move(listRule));
    }

    return threadgate;
}

AppBskyFeed::Postgate::SharedPtr PostMaster::createPostgate(const QString& uri, bool disableEmbedding, const QStringList& detachedEmbeddingUris)
{
    auto postgate = std::make_shared<AppBskyFeed::Postgate>();
    postgate->mPost = uri;
    postgate->mDisableEmbedding = disableEmbedding;
    postgate->mDetachedEmbeddingUris.assign(detachedEmbeddingUris.begin(), detachedEmbeddingUris.end());
    postgate->mCreatedAt = QDateTime::currentDateTimeUtc();

    return postgate;
}

QString PostMaster::createPostgateUri(const QString postUri)
{
    ATUri atUri(postUri);

    if (!atUri.isValid())
    {
        qWarning() << "Invalid at-uri:" << postUri;
        return {};
    }

    atUri.setCollection(AppBskyFeed::Postgate::TYPE);
    return atUri.toString();
}

void PostMaster::repost(const QString& uri, const QString& cid,
            const QString& viaUri, const QString& viaCid,
            const RepostSuccessCb& successCb, const ErrorCb& errorCb)
{
    const auto atUri = ATUri::createAtUri(uri, mPresence, errorCb);
    if (!atUri.isValid())
        return;

    AppBskyFeed::Repost repost;
    repost.mSubject = std::make_shared<ComATProtoRepo::StrongRef>();
    repost.mSubject->mUri = uri;
    repost.mSubject->mCid = cid;
    repost.mCreatedAt = QDateTime::currentDateTimeUtc();

    if (!viaUri.isEmpty())
    {
        repost.mVia = std::make_shared<ComATProtoRepo::StrongRef>();
        repost.mVia->mUri = viaUri;
        repost.mVia->mCid = viaCid;
    }

    const auto repostJson = repost.toJson();
    const QString& repo = mClient.getSession()->mDid;

    mClient.createRecord(repo, AppBskyFeed::Repost::TYPE, {}, repostJson, true,
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
          const QString& viaUri, const QString& viaCid,
          const LikeSuccessCb& successCb, const ErrorCb& errorCb)
{
    const auto atUri = ATUri::createAtUri(uri, mPresence, errorCb);
    if (!atUri.isValid())
        return;

    AppBskyFeed::Like like;
    like.mSubject = std::make_shared<ComATProtoRepo::StrongRef>();
    like.mSubject->mUri = uri;
    like.mSubject->mCid = cid;
    like.mCreatedAt = QDateTime::currentDateTimeUtc();

    if (!viaUri.isEmpty())
    {
        like.mVia = std::make_shared<ComATProtoRepo::StrongRef>();
        like.mVia->mUri = viaUri;
        like.mVia->mCid = viaCid;
    }

    const auto likeJson = like.toJson();
    const QString& repo = mClient.getSession()->mDid;

    mClient.createRecord(repo, AppBskyFeed::Like::TYPE, {}, likeJson, true,
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
                      const Client::SuccessCb& successCb, const ErrorCb& errorCb)
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

void PostMaster::checkRecordExists(const QString& uri, const QString& cid,
                                 const Client::SuccessCb& successCb, const ErrorCb& errorCb)
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

void PostMaster::getReposts(const AppBskyActor::ProfileViewBasic::SharedPtr& author,
                            std::optional<int> limit, const std::optional<QString>& cursor,
                            const GetRepostsSuccessCb& successCb, const ErrorCb& errorCb)
{
    Q_ASSERT(author);

    if (!limit)
        limit = MAX_GET_REPOSTS;

    if (limit < 1 || limit > MAX_GET_REPOSTS)
    {
        qWarning() << "Invalid limit:" << *limit;
        return;
    }

    mClient.listRecords(author->mDid, AppBskyFeed::Repost::TYPE, limit, cursor,
        [this, author, successCb, errorCb](ComATProtoRepo::ListRecordsOutput::SharedPtr output){
            if (output->mRecords.empty())
            {
                qDebug() << "No reposts:" << author->mDid;

                if (successCb)
                {
                    auto feed = std::make_shared<ATProto::AppBskyFeed::OutputFeed>();
                    feed->mCursor = output->mCursor;
                    successCb(feed);
                }

                return;
            }

            std::vector<AppBskyFeed::Repost::SharedPtr> reposts;

            for (const auto& record : output->mRecords)
            {
                try {
                    auto repost = AppBskyFeed::Repost::fromJson(record->mValue);
                    reposts.push_back(repost);
                } catch (InvalidJsonException& e) {
                    qWarning() << "Invalid repost:" << e.msg();
                    qInfo() << record->mValue;
                    continue;
                }
            }

            getRepostsContinue(author, output->mRecords, output->mCursor, successCb, errorCb);
        },
        [errorCb](const QString& err, const QString& msg){
            if (errorCb)
                errorCb(err, msg);
        });
}

void PostMaster::getRepostsContinue(const AppBskyActor::ProfileViewBasic::SharedPtr& author,
                                    const ATProto::ComATProtoRepo::Record::List& repostRecords,
                                    const std::optional<QString>& cursor,
                                    const GetRepostsSuccessCb& successCb, const ErrorCb& errorCb)
{
    std::vector<QString> uris;
    uris.reserve(repostRecords.size());
    std::unordered_map<QString, ATProto::ComATProtoRepo::Record::SharedPtr> recordMap;
    std::unordered_map<QString, AppBskyFeed::Repost::SharedPtr> repostMap;

    for (const auto& record : repostRecords)
    {
        try {
            auto repost = AppBskyFeed::Repost::fromJson(record->mValue);
            const QString& uri = repost->mSubject->mUri;
            uris.push_back(uri);
            recordMap[uri] = record;
            repostMap[uri] = repost;
        } catch (InvalidJsonException& e) {
            qWarning() << "Invalid repost:" << e.msg();
            qInfo() << record->mValue;
            continue;
        }
    }

    if (uris.empty())
    {
        qWarning() << "No URIs";

        if (successCb)
        {
            auto feed = std::make_shared<ATProto::AppBskyFeed::OutputFeed>();
            feed->mCursor = cursor;
            successCb(feed);
        }

        return;
    }

    mClient.getPosts(uris,
        [recordMap, repostMap, author, cursor, successCb](AppBskyFeed::PostView::List posts){
            if (!successCb)
                return;

            auto feed = std::make_shared<ATProto::AppBskyFeed::OutputFeed>();
            feed->mCursor = cursor;

            for (const auto& post : posts)
            {
                auto feedViewPost = std::make_shared<ATProto::AppBskyFeed::FeedViewPost>();
                feedViewPost->mPost = post;

                auto reason = std::make_shared<AppBskyFeed::ReasonRepost>();
                reason->mBy = author;

                const auto repostIt = repostMap.find(post->mUri);

                if (repostIt == repostMap.end())
                {
                    qWarning() << "URI missing om repostMap:" << post->mUri;
                    continue;
                }

                reason->mIndexedAt = repostIt->second->mCreatedAt;

                const auto recordIt = recordMap.find(post->mUri);

                if (recordIt == recordMap.end())
                {
                    qWarning() << "URI missing in recordMap:" << post->mUri;
                    continue;
                }

                reason->mUri = recordIt->second->mUri;
                reason->mCid = recordIt->second->mCid;

                feedViewPost->mReason = reason;
                feed->mFeed.push_back(feedViewPost);
            }

            successCb(feed);
        },
        [errorCb](const QString& err, const QString& msg){
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

void PostMaster::continueGetPost(const ATUri& atUri, AppBskyActor::ProfileViewDetailed::SharedPtr author, const PostCb& successCb)
{
    mClient.getRecord(atUri.getAuthority(), atUri.getCollection(), atUri.getRkey(), {},
        [successCb, author](ComATProtoRepo::Record::SharedPtr record){
            try {
                auto post = AppBskyFeed::Record::Post::fromJson(record->mValue);

                if (successCb)
                    successCb(record->mUri, record->mCid.value_or(""), std::move(post), author);
            } catch (InvalidJsonException& e) {
                qWarning() << e.msg();
            }
        },
        [](const QString& err, const QString& msg){
            qDebug() << err << " - " << msg;
        });
}

void PostMaster::getFeed(const QString& httpsUri, const FeedCb& successCb)
{
    auto atUri = ATUri::fromHttpsFeedUri(httpsUri);

    if (!atUri.isValid())
        return;

    mClient.getProfile(atUri.getAuthority(),
        [this, presence=getPresence(), atUri, successCb](auto profile){
            if (!presence)
                return;

            ATUri newUri(atUri);
            newUri.setAuthority(profile->mDid);
            newUri.setAuthorityIsHandle(false);
            continueGetFeed(newUri, successCb);
        },
        [](const QString& err, const QString& msg){
            qDebug() << err << " - " << msg;
        });
}

void PostMaster::continueGetFeed(const ATUri& atUri, const FeedCb& successCb)
{
    mClient.getFeedGenerator(atUri.toString(),
        [successCb](auto output){
            try {
                if (successCb)
                    successCb(std::move(output->mView));
            } catch (InvalidJsonException& e) {
                qWarning() << e.msg();
            }
        },
        [](const QString& err, const QString& msg){
            qDebug() << err << " - " << msg;
        });
}

void PostMaster::getList(const QString& httpsUri, const ListCb& successCb)
{
    auto atUri = ATUri::fromHttpsListUri(httpsUri);

    if (!atUri.isValid())
        return;

    mClient.getProfile(atUri.getAuthority(),
        [this, presence=getPresence(), atUri, successCb](auto profile){
            if (!presence)
                return;

            ATUri newUri(atUri);
            newUri.setAuthority(profile->mDid);
            newUri.setAuthorityIsHandle(false);
            continueGetList(newUri, successCb);
        },
        [](const QString& err, const QString& msg){
            qDebug() << err << " - " << msg;
        });
}

void PostMaster::continueGetList(const ATUri& atUri, const ListCb& successCb)
{
    mClient.getList(atUri.toString(), 1, {},
        [successCb](auto output){
            try {
                if (successCb)
                    successCb(std::move(output->mList));
            } catch (InvalidJsonException& e) {
                qWarning() << e.msg();
            }
        },
        [](const QString& err, const QString& msg){
            qDebug() << err << " - " << msg;
        });
}

void PostMaster::getPostgate(const QString& postUri, const PostgateCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Get postgate:" << postUri;
    const auto atUri = ATUri::createAtUri(postUri, mPresence, errorCb);

    if (!atUri.isValid())
        return;

    mClient.getRecord(atUri.getAuthority(), AppBskyFeed::Postgate::TYPE, atUri.getRkey(), {},
        [successCb, errorCb](auto record){
            qDebug() << "Got postgate:" << record->mValue;

            try {
                auto postgate = AppBskyFeed::Postgate::fromJson(record->mValue);

                if (successCb)
                    successCb(std::move(postgate));
            } catch (InvalidJsonException& e) {
                qWarning() << e.msg();

                if (errorCb)
                    errorCb("InvalidJsonException", e.msg());
            }
        },
        [errorCb](const QString& err, const QString& msg){
            qDebug() << err << " - " << msg;

            if (errorCb)
                errorCb(err, msg);
        });
}

AppBskyFeed::PostReplyRef::SharedPtr PostMaster::createReplyRef(const QString& replyToUri, const QString& replyToCid,
                                              const QString& replyRootUri, const QString& replyRootCid)
{
    if (replyToUri.isEmpty() || replyToCid.isEmpty())
        return nullptr;

    auto replyRef = std::make_shared<ATProto::AppBskyFeed::PostReplyRef>();

    replyRef->mParent = std::make_shared<ATProto::ComATProtoRepo::StrongRef>();
    replyRef->mParent->mUri = replyToUri;
    replyRef->mParent->mCid = replyToCid;

    replyRef->mRoot = std::make_shared<ATProto::ComATProtoRepo::StrongRef>();
    replyRef->mRoot->mUri = replyRootUri.isEmpty() ? replyToUri : replyRootUri;
    replyRef->mRoot->mCid = replyRootCid.isEmpty() ? replyToCid : replyRootCid;

    return replyRef;
}

AppBskyFeed::Record::Post::SharedPtr PostMaster::createPostWithoutFacets(
    const QString& text, const QString& language, AppBskyFeed::PostReplyRef::SharedPtr replyRef)
{
    auto post = std::make_shared<AppBskyFeed::Record::Post>();
    post->mCreatedAt = QDateTime::currentDateTimeUtc();
    post->mText = text;

    if (!language.isEmpty())
        post->mLanguages.push_back(language);

    post->mReply = std::move(replyRef);
    return post;
}

void PostMaster::createPost(const QString& text, const QString& language,
                            AppBskyFeed::PostReplyRef::SharedPtr replyRef,
                            const std::vector<RichTextMaster::ParsedMatch>& embeddedLinks,
                            const PostCreatedCb& cb)
{
    Q_ASSERT(cb);
    auto post = std::make_shared<AppBskyFeed::Record::Post>();
    post->mCreatedAt = QDateTime::currentDateTimeUtc();

    if (!language.isEmpty())
        post->mLanguages.push_back(language);

    post->mReply = std::move(replyRef);
    auto facets = RichTextMaster::parseFacets(text);
    RichTextMaster::insertEmbeddedLinksToFacets(embeddedLinks, facets);

    mRichTextMaster.resolveFacets(text, facets, 0, true,
        [post, cb](const QString& richText, AppBskyRichtext::Facet::List resolvedFacets){
            post->mText = richText;
            post->mFacets = std::move(resolvedFacets);
            cb(post);
        });
}

void PostMaster::addQuoteToPost(AppBskyFeed::Record::Post& post, const QString& quoteUri, const QString& quoteCid)
{
    auto ref = std::make_shared<ComATProtoRepo::StrongRef>();
    ref->mUri = quoteUri;
    ref->mCid = quoteCid;

    Q_ASSERT(!post.mEmbed);
    post.mEmbed = std::make_shared<AppBskyEmbed::Embed>();
    post.mEmbed->mType = AppBskyEmbed::EmbedType::RECORD;
    post.mEmbed->mEmbed = std::make_shared<AppBskyEmbed::Record>();

    auto& record = std::get<AppBskyEmbed::Record::SharedPtr>(post.mEmbed->mEmbed);
    record->mRecord = std::move(ref);
}

void PostMaster::addLabelsToPost(AppBskyFeed::Record::Post& post, const QStringList& labels)
{
    if (labels.isEmpty())
        return;

    if (!post.mLabels)
        post.mLabels = std::make_shared<ComATProtoLabel::SelfLabels>();

    for (const auto& label : labels)
    {
        auto l = std::make_shared<ComATProtoLabel::SelfLabel>();
        l->mVal = label;
        post.mLabels->mValues.push_back(std::move(l));
    }
}

void PostMaster::addImageToPost(AppBskyFeed::Record::Post& post, Blob::SharedPtr blob, int width, int height, const QString& altText)
{
    if (!post.mEmbed)
    {
        post.mEmbed = std::make_shared<AppBskyEmbed::Embed>();
        post.mEmbed->mType = AppBskyEmbed::EmbedType::IMAGES;
        post.mEmbed->mEmbed = std::make_shared<AppBskyEmbed::Images>();
    }
    else if (post.mEmbed->mType == AppBskyEmbed::EmbedType::RECORD)
    {
        auto& record = std::get<AppBskyEmbed::Record::SharedPtr>(post.mEmbed->mEmbed);
        auto ref = std::move(record->mRecord);
        post.mEmbed->mType = AppBskyEmbed::EmbedType::RECORD_WITH_MEDIA;
        post.mEmbed->mEmbed = std::make_shared<AppBskyEmbed::RecordWithMedia>();

        auto& recordWithMedia = std::get<AppBskyEmbed::RecordWithMedia::SharedPtr>(post.mEmbed->mEmbed);
        recordWithMedia->mRecord = std::make_shared<AppBskyEmbed::Record>();
        recordWithMedia->mRecord->mRecord = std::move(ref);
        recordWithMedia->mMediaType = AppBskyEmbed::EmbedType::IMAGES;
        recordWithMedia->mMedia = std::make_shared<AppBskyEmbed::Images>();
    }

    auto image = std::make_shared<AppBskyEmbed::Image>();
    image->mImage = std::move(blob);
    image->mAlt = altText;
    image->mAspectRatio = std::make_shared<AppBskyEmbed::AspectRatio>();
    image->mAspectRatio->mWidth = width;
    image->mAspectRatio->mHeight = height;

    AppBskyEmbed::Images* images = nullptr;
    if (post.mEmbed->mType == AppBskyEmbed::EmbedType::IMAGES)
    {
        images = std::get<AppBskyEmbed::Images::SharedPtr>(post.mEmbed->mEmbed).get();
    }
    else
    {
        Q_ASSERT(post.mEmbed->mType == AppBskyEmbed::EmbedType::RECORD_WITH_MEDIA);
        auto& recordWithMedia = std::get<AppBskyEmbed::RecordWithMedia::SharedPtr>(post.mEmbed->mEmbed);
        images = std::get<AppBskyEmbed::Images::SharedPtr>(recordWithMedia->mMedia).get();
    }

    Q_ASSERT(images);
    images->mImages.push_back(std::move(image));
}

void PostMaster::addExternalToPost(AppBskyFeed::Record::Post& post, const QString& link,
                               const QString& title, const QString& description, Blob::SharedPtr blob)
{
    AppBskyEmbed::External* embed = nullptr;

    if (!post.mEmbed)
    {
        post.mEmbed = std::make_shared<AppBskyEmbed::Embed>();
        post.mEmbed->mType = AppBskyEmbed::EmbedType::EXTERNAL;
        post.mEmbed->mEmbed = std::make_shared<AppBskyEmbed::External>();
        embed = std::get<AppBskyEmbed::External::SharedPtr>(post.mEmbed->mEmbed).get();
    }
    else
    {
        Q_ASSERT(post.mEmbed->mType == AppBskyEmbed::EmbedType::RECORD);
        auto& record = std::get<AppBskyEmbed::Record::SharedPtr>(post.mEmbed->mEmbed);
        auto ref = std::move(record->mRecord);
        post.mEmbed->mType = AppBskyEmbed::EmbedType::RECORD_WITH_MEDIA;
        post.mEmbed->mEmbed = std::make_shared<AppBskyEmbed::RecordWithMedia>();

        auto& recordWithMedia = std::get<AppBskyEmbed::RecordWithMedia::SharedPtr>(post.mEmbed->mEmbed);
        recordWithMedia->mRecord = std::make_shared<AppBskyEmbed::Record>();
        recordWithMedia->mRecord->mRecord = std::move(ref);
        recordWithMedia->mMediaType = AppBskyEmbed::EmbedType::EXTERNAL;
        recordWithMedia->mMedia = std::make_shared<AppBskyEmbed::External>();

        embed = std::get<AppBskyEmbed::External::SharedPtr>(recordWithMedia->mMedia).get();
    }

    Q_ASSERT(embed);
    embed->mExternal = std::make_shared<AppBskyEmbed::ExternalExternal>();
    embed->mExternal->mUri = link;
    embed->mExternal->mTitle = title;
    embed->mExternal->mDescription = description;
    embed->mExternal->mThumb = std::move(blob);
}

void PostMaster::addVideoToPost(AppBskyFeed::Record::Post& post, Blob::SharedPtr blob, int width, int height, const QString& altText)
{
    // Post duplication
    AppBskyEmbed::Video* embed = nullptr;

    if (!post.mEmbed)
    {
        post.mEmbed = std::make_shared<AppBskyEmbed::Embed>();
        post.mEmbed->mType = AppBskyEmbed::EmbedType::VIDEO;
        post.mEmbed->mEmbed = std::make_shared<AppBskyEmbed::Video>();
        embed = std::get<AppBskyEmbed::Video::SharedPtr>(post.mEmbed->mEmbed).get();
    }
    else if (post.mEmbed->mType == AppBskyEmbed::EmbedType::RECORD)
    {
        auto& record = std::get<AppBskyEmbed::Record::SharedPtr>(post.mEmbed->mEmbed);
        auto ref = std::move(record->mRecord);
        post.mEmbed->mType = AppBskyEmbed::EmbedType::RECORD_WITH_MEDIA;
        post.mEmbed->mEmbed = std::make_shared<AppBskyEmbed::RecordWithMedia>();

        auto& recordWithMedia = std::get<AppBskyEmbed::RecordWithMedia::SharedPtr>(post.mEmbed->mEmbed);
        recordWithMedia->mRecord = std::make_shared<AppBskyEmbed::Record>();
        recordWithMedia->mRecord->mRecord = std::move(ref);
        recordWithMedia->mMediaType = AppBskyEmbed::EmbedType::VIDEO;
        recordWithMedia->mMedia = std::make_shared<AppBskyEmbed::Video>();

        embed = std::get<AppBskyEmbed::Video::SharedPtr>(recordWithMedia->mMedia).get();
    }

    Q_ASSERT(embed);
    embed->mVideo = blob;

    if (!altText.isEmpty())
        embed->mAlt = altText;

    if (width > 0 && height > 0)
    {
        embed->mAspectRatio = std::make_shared<AppBskyEmbed::AspectRatio>();
        embed->mAspectRatio->mWidth = width;
        embed->mAspectRatio->mHeight = height;
    }
}

void PostMaster::addVideoToPost(AppBskyFeed::Record::Post::SharedPtr post, const AppBskyVideo::JobStatus& jobStatus,
                                int width, int height, const QString& altText,
                                const SuccessCb& successCb, const ErrorCb& errorCb, const ProgressCb& progressCb)
{
    switch (jobStatus.mState)
    {
    case AppBskyVideo::JobStatusState::JOB_STATE_COMPLETED:
    {
        if (jobStatus.mBlob)
        {
            addVideoToPost(*post, jobStatus.mBlob, width, height, altText);

            if (successCb)
                successCb();
        }
        else
        {
            qWarning() << "Blob missing from job status";
            if (errorCb)
                errorCb("UpdloadError", "Video blob missing");
        }

        break;
    }
    case AppBskyVideo::JobStatusState::JOB_STATE_FAILED:
        if (errorCb)
            errorCb(jobStatus.mError.value_or("UploadError"), jobStatus.mMessage.value_or("Job failed"));

        break;
    case AppBskyVideo::JobStatusState::JOB_STATE_INPROG:
        qDebug() << "Upload in progress, job:" << jobStatus.mJobId << "progress:" << jobStatus.mProgress.value_or(-1);

        if (progressCb)
        {
            const QString status = jobStatus.mRawState.startsWith("JOB_STATE_") ? jobStatus.mRawState.sliced(10) : jobStatus.mRawState;
            progressCb(status, jobStatus.mProgress);
        }

        QTimer::singleShot(1500, &mPresence, [this, post, jobId=jobStatus.mJobId, width, height, altText, successCb, errorCb, progressCb]{
            checkVideoUploadStatus(post, jobId, width, height, altText, successCb, errorCb, progressCb); });
        break;
    }
}

void PostMaster::checkVideoUploadStatus(AppBskyFeed::Record::Post::SharedPtr post, const QString jobId, int width, int height, const QString& altText,
                                        const SuccessCb& successCb, const ErrorCb& errorCb, const ProgressCb& progressCb)
{
    mClient.getVideoJobStatus(jobId,
        [this, presence=getPresence(), post, width, height, altText, successCb, errorCb, progressCb](AppBskyVideo::JobStatusOutput::SharedPtr output){
            if (!presence)
                return;

            addVideoToPost(post, *output->mJobStatus, width, height, altText, successCb, errorCb, progressCb);
        },
        [errorCb](const QString& err, const QString& msg){
            qDebug() << err << " - " << msg;

            if (errorCb)
                errorCb(err, msg);
        });
}

void PostMaster::sendInteractionShowMoreLikeThis(const QString& postUri, const QString& feedDid, const QString& feedContext,
                                     const SuccessCb& successCb, const ErrorCb& errorCb)
{
    sendInteraction(postUri, feedDid, feedContext,
                    AppBskyFeed::Interaction::EventType::RequestMore,
                    successCb, errorCb);
}

void PostMaster::sendInteractionShowLessLikeThis(const QString& postUri, const QString& feedDid, const QString& feedContext,
                                                 const SuccessCb& successCb, const ErrorCb& errorCb)
{
    sendInteraction(postUri, feedDid, feedContext,
                    AppBskyFeed::Interaction::EventType::RequestLess,
                    successCb, errorCb);
}

void PostMaster::sendInteraction(const QString& postUri, const QString& feedDid, const QString& feedContext,
                     AppBskyFeed::Interaction::EventType event, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Send interaction, postUri:" << postUri << "feedDid:" << feedDid << "event:" << AppBskyFeed::Interaction::eventTypeToString(event);
    auto interaction = std::make_shared<AppBskyFeed::Interaction>();
    interaction->mEvent = event;
    interaction->mItem = postUri;

    if (!feedContext.isEmpty())
        interaction->mFeedContext = feedContext;

    mClient.sendInteractions({interaction}, feedDid,
        [successCb]{
            if (successCb)
                    successCb();
        },
        [errorCb](const QString& err, const QString& msg){
            qDebug() << err << " - " << msg;

            if (errorCb)
                errorCb(err, msg);
        });
}

}
