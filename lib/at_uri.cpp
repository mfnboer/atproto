// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "at_uri.h"
#include <QStringList>
#include <QDebug>

namespace ATProto {

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
