// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "at_uri.h"
#include "at_regex.h"
#include <QRegularExpression>
#include <QStringList>
#include <QTimer>

namespace ATProto {

static ATUri fromHttpsUri(const QString& uri, const QRegularExpression& reHandleHttps,
                          const QRegularExpression& reDidHttps, char const* collection)
{
    bool authorityIsHandle = true;
    auto match = reHandleHttps.match(uri);

    if (!match.hasMatch())
    {
        match = reDidHttps.match(uri);

        if (!match.hasMatch())
            return {};

        authorityIsHandle = false;
    }

    ATUri atUri;
    atUri.setAuthority(match.captured("authority"));
    atUri.setCollection(collection);
    atUri.setRKey(match.captured("rkey"));
    atUri.setAuthorityIsHandle(authorityIsHandle);

    return atUri;
}

ATUri ATUri::fromHttpsPostUri(const QString& uri)
{
    static const QRegularExpression reHandleHttps(
        QString(R"(^https://bsky.app/profile/(?<authority>%1)/post/(?<rkey>%2)$)").arg(
            ATRegex::HANDLE.pattern(),
            ATRegex::RKEY.pattern()));

    static const QRegularExpression reDidHttps(
        QString(R"(^https://bsky.app/profile/(?<authority>%1)/post/(?<rkey>%2)$)").arg(
            ATRegex::DID.pattern(),
            ATRegex::RKEY.pattern()));

    return fromHttpsUri(uri, reHandleHttps, reDidHttps, COLLECTION_FEED_POST);
}

ATUri ATUri::fromHttpsFeedUri(const QString& uri)
{
    static const QRegularExpression reHandleHttps(
        QString(R"(^https://bsky.app/profile/(?<authority>%1)/feed/(?<rkey>%2)$)").arg(
            ATRegex::HANDLE.pattern(),
            ATRegex::RKEY.pattern()));

    static const QRegularExpression reDidHttps(
        QString(R"(^https://bsky.app/profile/(?<authority>%1)/feed/(?<rkey>%2)$)").arg(
            ATRegex::DID.pattern(),
            ATRegex::RKEY.pattern()));

    return fromHttpsUri(uri, reHandleHttps, reDidHttps, COLLECTION_FEED_GENERATOR);
}

ATUri ATUri::fromHttpsListUri(const QString& uri)
{
    static const QRegularExpression reHandleHttps(
        QString(R"(^https://bsky.app/profile/(?<authority>%1)/lists/(?<rkey>%2)$)").arg(
            ATRegex::HANDLE.pattern(),
            ATRegex::RKEY.pattern()));

    static const QRegularExpression reDidHttps(
        QString(R"(^https://bsky.app/profile/(?<authority>%1)/lists/(?<rkey>%2)$)").arg(
            ATRegex::DID.pattern(),
            ATRegex::RKEY.pattern()));

    return fromHttpsUri(uri, reHandleHttps, reDidHttps, COLLECTION_GRAPH_LIST);
}

ATUri ATUri::fromHttpsStarterPackUri(const QString& uri)
{
    static const QRegularExpression reHandleHttps(
        QString(R"(^https://bsky.app/starter-pack/(?<authority>%1)/(?<rkey>%2)$)").arg(
            ATRegex::HANDLE.pattern(),
            ATRegex::RKEY.pattern()));

    static const QRegularExpression reDidHttps(
        QString(R"(^https://bsky.app/starter-pack/(?<authority>%1)/(?<rkey>%2)$)").arg(
            ATRegex::DID.pattern(),
            ATRegex::RKEY.pattern()));

    return fromHttpsUri(uri, reHandleHttps, reDidHttps, COLLECTION_GRAPH_STARTERPACK);
}

ATUri ATUri::createAtUri(const QString& uri, const QObject& presence, const ErrorCb& errorCb)
{
    auto atUri = ATUri(uri);

    if (!atUri.isValid() && errorCb)
        QTimer::singleShot(0, &presence, [errorCb, uri]{ errorCb("InvalidUri", "Invalid at-uri: " + uri); });

    return atUri;
}

ATUri::ATUri(const QString& uri)
{
    if (!uri.startsWith("at://"))
    {
        qDebug() << "Invalid at-uri:" << uri;
        return;
    }

    auto uriParts = uri.split('/', Qt::SkipEmptyParts);
    if (uriParts.size() != 4)
    {
        qDebug() << "Invalid at-uri:" << uri;
        return;
    }

    mAuthority = uriParts[1];
    mCollection = uriParts[2];
    mRkey = uriParts[3];
    mAuthorityIsHandle = !mAuthority.startsWith("did:");
}

ATUri::ATUri(const QString& authority, const QString& collection, const QString& rKey) :
    mAuthority(authority),
    mCollection(collection),
    mRkey(rKey)
{
}

QString ATUri::toString() const
{
    return QString("at://%1/%2/%3").arg(mAuthority, mCollection, mRkey);
}

QString ATUri::toHttpsUri() const
{
    if (mCollection == COLLECTION_FEED_POST)
        return QString("https://bsky.app/profile/%1/post/%2").arg(mAuthority, mRkey);
    if (mCollection == COLLECTION_FEED_GENERATOR)
        return QString("https://bsky.app/profile/%1/feed/%2").arg(mAuthority, mRkey);
    if (mCollection == COLLECTION_GRAPH_LIST)
        return QString("https://bsky.app/profile/%1/lists/%2").arg(mAuthority, mRkey);
    if (mCollection == COLLECTION_GRAPH_STARTERPACK)
        return QString("https://bsky.app/starter-pack/%1/%2").arg(mAuthority, mRkey);
    if (mCollection == COLLECTION_ACTOR_PROFILE)
        return QString("https://bsky.app/profile/%1").arg(mAuthority);

    qWarning() << "Unknown collection:" << mCollection;
    return {};
}

bool ATUri::isValid() const
{
    return !mAuthority.isEmpty();
}

}
