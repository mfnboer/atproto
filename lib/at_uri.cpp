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
}

QString ATUri::toString() const
{
    return QString("at://%1/%2/%3").arg(mAuthority, mCollection, mRkey);
}

bool ATUri::isValid() const
{
    return !mAuthority.isEmpty();
}

}
