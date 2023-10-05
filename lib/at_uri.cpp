// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "at_uri.h"
#include <QRegularExpression>
#include <QStringList>
#include <QTimer>

namespace ATProto {

ATUri ATUri::fromHttpsPostUri(const QString& uri)
{
    static const QRegularExpression reHttps(R"(^https://bsky.app/profile/([a-zA-Z0-9-\._~]+)/post/([a-zA-Z0-9\.-_~]+)$)");

    auto match = reHttps.match(uri);

    if (!match.hasMatch())
        return {};

    ATUri atUri;
    atUri.mAuthority = match.captured(1);
    atUri.mCollection = "app.bsky.feed.post";
    atUri.mRkey = match.captured(2);
    atUri.mAuthorityIsHandle = true;

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

const bool ATUri::isValid() const
{
    return !mAuthority.isEmpty();
}

}
