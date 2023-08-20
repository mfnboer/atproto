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
        mBsky->getAuthorFeed(author, 10, {},
            [this](auto&& feed) {
                for (const auto& post : feed->mFeed)
                {
                    const auto& postView = post->mPost;
                    const auto& record = postView->mRecord;
                    qDebug() << "Post:" << std::get<AppBskyFeed::Record::Post::Ptr>(record)->mText;
                }
            },
            [](const QString& err){ qDebug() << "getAuthorFeed FAILED:" << err; }
        );
    }

    std::unique_ptr<ATProto::Client> mBsky;
    AppBskyActor::ProfileViewDetailed mProfile;
};

}
