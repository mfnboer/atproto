// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "client.h"
#include <QObject>
#include <QtQmlIntegration>

namespace ATProto {

class ATProtoTest : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString avatar READ getAvatar NOTIFY profileChanged)
    Q_PROPERTY(QString banner READ getBanner NOTIFY profileChanged)
    Q_PROPERTY(QString displayName READ getDisplayName NOTIFY profileChanged)
    Q_PROPERTY(QString description READ getDescription NOTIFY profileChanged)
    QML_ELEMENT

public:
    explicit ATProtoTest(QObject* parent = nullptr);

    Q_INVOKABLE void login(const QString user, QString password, const QString host)
    {
        auto xrpc = std::make_unique<Xrpc::Client>(host);
        mBsky = std::make_unique<ATProto::Client>(std::move(xrpc));
        mBsky->createSession(user, password,
            [this, user]{
                getProfile(user);
                getAuthorFeed(user);
            },
            [](const QString& err){ qDebug() << "LOGIN FAILED:" << err; });
    }

    QString getAvatar() const { return mProfile.mAvatar.value_or(QString()); }
    QString getBanner() const { return mProfile.mBanner.value_or(QString()); }
    QString getDisplayName() const { return mProfile.mDisplayName.value_or(QString()); }
    QString getDescription() const { return mProfile.mDescription.value_or(QString()); }

signals:
    void profileChanged();

private:
    void getProfile(const QString& user)
    {
        mBsky->getProfile(user,
            [this](auto&& profile){
                mProfile = *profile;
                emit profileChanged();
            },
            [](const QString& err){ qDebug() << "getProfile FAILED:" << err; });
    }

    void getAuthorFeed(const QString& author)
    {
        mBsky->getAuthorFeed(author, 50, {},
            [this](auto&& feed) {
                for (const AppBskyFeed::FeedViewPost::Ptr& feedEntry : feed->mFeed)
                {
                    const auto& postView = feedEntry->mPost;
                    Q_ASSERT(postView->mRecordType == RecordType::APP_BSKY_FEED_POST);
                    const auto& record = postView->mRecord;
                    const auto& post = std::get<AppBskyFeed::Record::Post::Ptr>(record);
                    qDebug() << "Post:" << post->mCreatedAt << post->mText;

                    if (postView->mEmbed)
                        logEmbed(*postView->mEmbed);

                    if (feedEntry->mReply)
                    {
                        qDebug() << "REPLY TO:";
                        logPostView(*feedEntry->mReply->mParent);
                        qDebug() << "ROOT:";
                        logPostView(*feedEntry->mReply->mRoot);
                    }
                }
            },
            [](const QString& err){ qDebug() << "getAuthorFeed FAILED:" << err; }
        );
    }

    void logEmbed(const AppBskyEmbed::Embed& embed)
    {
        switch (embed.mType)
        {
        case AppBskyEmbed::EmbedType::IMAGES_VIEW:
        {
            const auto& view = std::get<AppBskyEmbed::ImagesView::Ptr>(embed.mEmbed);
            for (const auto& image : view->mImages)
            {
                qDebug() << "Image:" << image->mFullSize;
                qDebug() << "Alt:" << image->mAlt;
            }
            break;
        }
        case AppBskyEmbed::EmbedType::EXTERNAL_VIEW:
        {
            const auto& view = std::get<AppBskyEmbed::ExternalView::Ptr>(embed.mEmbed);
            qDebug() << "External:" << view->mExternal->mTitle;
            break;
        }
        case AppBskyEmbed::EmbedType::RECORD_VIEW:
        {
            const auto& view = std::get<AppBskyEmbed::RecordView::Ptr>(embed.mEmbed);
            switch (view->mRecordType)
            {
            case RecordType::APP_BSKY_EMBED_RECORD_VIEW_RECORD:
                logRecordViewRecord(*std::get<AppBskyEmbed::RecordViewRecord::Ptr>(view->mRecord));
                break;
            case RecordType::APP_BSKY_EMBED_RECORD_VIEW_NOT_FOUND:
                qDebug() << "RECORD NOT FOUND";
                break;
            case RecordType::APP_BSKY_EMBED_RECORD_VIEW_BLOCKED:
                qDebug() << "RECORD BLOCKED";
                break;
            default:
                qDebug() << "UNKNOW RECORD";
                break;
            }

            break;
        }
        default:
            qDebug() << "Other embed";
            break;
        }
    }

    void logRecordViewRecord(const AppBskyEmbed::RecordViewRecord& recordViewRecord)
    {
        qDebug() << "RECORD:";
        const auto& post = std::get<AppBskyFeed::Record::Post::Ptr>(recordViewRecord.mValue);
        qDebug() << "  Author:" << recordViewRecord.mAuthor->mHandle;
        qDebug() << "  Post:" << post->mCreatedAt << post->mText;
    }

    void logPostView(const AppBskyFeed::PostView& view)
    {
        qDebug() << "  cid:" << view.mCid;
        qDebug() << "  Author:" << view.mAuthor->mHandle;
        const auto& post = std::get<AppBskyFeed::Record::Post::Ptr>(view.mRecord);
        qDebug() << "  Post:" << post->mCreatedAt << post->mText;
    }

    std::unique_ptr<ATProto::Client> mBsky;
    AppBskyActor::ProfileViewDetailed mProfile;
};

}
