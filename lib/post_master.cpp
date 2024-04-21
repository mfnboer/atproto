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

void PostMaster::addThreadgate(const QString& uri, bool allowMention, bool allowFollowing, const QStringList& allowLists,
                   const Client::SuccessCb& successCb, const ErrorCb& errorCb)
{
    const auto atUri = ATUri::createAtUri(uri, mPresence, errorCb);
    if (!atUri.isValid())
        return;

    auto threadgate = createThreadgate(uri, allowMention, allowFollowing, allowLists);
    QJsonObject threadgateJson = threadgate->toJson();
    qDebug() << "Add threadgate:" << threadgateJson;
    const QString& repo = mClient.getSession()->mDid;
    const QString collection = threadgateJson["$type"].toString();

    mClient.createRecord(repo, collection, atUri.getRkey(), threadgateJson, true,
        [successCb](auto){
            if (successCb)
                successCb();
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

AppBskyFeed::Threadgate::Ptr PostMaster::createThreadgate(const QString& uri, bool allowMention,
        bool allowFollowing, const QStringList& allowLists)
{
    auto threadgate = std::make_unique<AppBskyFeed::Threadgate>();
    threadgate->mPost = uri;
    threadgate->mAllowMention = allowMention;
    threadgate->mAllowFollowing = allowFollowing;
    threadgate->mCreatedAt = QDateTime::currentDateTimeUtc();

    for (const auto& listUri : allowLists)
    {
        auto listRule = std::make_unique<AppBskyFeed::ThreadgateListRule>();
        listRule->mList = listUri;
        threadgate->mAllowList.push_back(std::move(listRule));
    }

    return threadgate;
}

void PostMaster::repost(const QString& uri, const QString& cid,
            const RepostSuccessCb& successCb, const ErrorCb& errorCb)
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

    mClient.createRecord(repo, collection, {}, repostJson, true,
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
          const LikeSuccessCb& successCb, const ErrorCb& errorCb)
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

    mClient.createRecord(repo, collection, {}, likeJson, true,
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

AppBskyFeed::PostReplyRef::Ptr PostMaster::createReplyRef(const QString& replyToUri, const QString& replyToCid,
                                              const QString& replyRootUri, const QString& replyRootCid)
{
    if (replyToUri.isEmpty() || replyToCid.isEmpty())
        return nullptr;

    auto replyRef = std::make_unique<ATProto::AppBskyFeed::PostReplyRef>();

    replyRef->mParent = std::make_unique<ATProto::ComATProtoRepo::StrongRef>();
    replyRef->mParent->mUri = replyToUri;
    replyRef->mParent->mCid = replyToCid;

    replyRef->mRoot = std::make_unique<ATProto::ComATProtoRepo::StrongRef>();
    replyRef->mRoot->mUri = replyRootUri.isEmpty() ? replyToUri : replyRootUri;
    replyRef->mRoot->mCid = replyRootCid.isEmpty() ? replyToCid : replyRootCid;

    return replyRef;
}

AppBskyFeed::Record::Post::Ptr PostMaster::createPostWithoutFacets(
    const QString& text, const QString& language, AppBskyFeed::PostReplyRef::Ptr replyRef)
{
    auto post = std::make_unique<AppBskyFeed::Record::Post>();
    post->mCreatedAt = QDateTime::currentDateTimeUtc();
    post->mText = text;

    if (!language.isEmpty())
        post->mLanguages.push_back(language);

    post->mReply = std::move(replyRef);
    return post;
}

void PostMaster::createPost(const QString& text, const QString& language, AppBskyFeed::PostReplyRef::Ptr replyRef, const PostCreatedCb& cb)
{
    Q_ASSERT(cb);
    auto post = std::make_shared<AppBskyFeed::Record::Post>();
    post->mCreatedAt = QDateTime::currentDateTimeUtc();

    if (!language.isEmpty())
        post->mLanguages.push_back(language);

    post->mReply = std::move(replyRef);
    auto facets = RichTextMaster::parseFacets(text);

    mRichTextMaster.resolveFacets(text, facets, 0,
        [post, cb](const QString& richText, AppBskyRichtext::FacetList resolvedFacets){
            post->mText = richText;
            post->mFacets = std::move(resolvedFacets);
            cb(post);
        });
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

void PostMaster::addLabelsToPost(AppBskyFeed::Record::Post& post, const QStringList& labels)
{
    if (labels.isEmpty())
        return;

    if (!post.mLabels)
        post.mLabels = std::make_unique<ComATProtoLabel::SelfLabels>();

    for (const auto& label : labels)
    {
        auto l = std::make_unique<ComATProtoLabel::SelfLabel>();
        l->mVal = label;
        post.mLabels->mValues.push_back(std::move(l));
    }
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

}
