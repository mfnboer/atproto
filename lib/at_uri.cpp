// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "at_uri.h"
#include "at_regex.h"
#include <QRegularExpression>
#include <QStringList>
#include <QTimer>

namespace ATProto {

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
    atUri.mAuthority = match.captured("authority");
    atUri.mCollection = "app.bsky.feed.post";
    atUri.mRkey = match.captured("rkey");
    atUri.mAuthorityIsHandle = authorityIsHandle;

    return atUri;
}

ATUri ATUri::createAtUri(const QString& uri, const QObject& presence, const ErrorCb& errorCb)
{
    auto atUri = ATUri(uri);

    if (!atUri.isValid() && errorCb)
        QTimer::singleShot(0, &presence, [errorCb, uri]{ errorCb("Invalid at-uri: " + uri); });

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
