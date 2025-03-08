// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "identity_resolver.h"
#include "at_regex.h"

namespace ATProto {

IdentityResolver::IdentityResolver()
{
    mNetwork.setAutoDeleteReplies(true);
    mNetwork.setTransferTimeout(10000);
}

void IdentityResolver::resolveHandle(const QString& handle, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Resolve handle:" << handle;
    const QString lookupName = getDnsLookupName(handle);
    mDns = std::make_unique<QDnsLookup>(QDnsLookup::TXT, lookupName);
    connect(mDns.get(), &QDnsLookup::finished, this, [this, handle, successCb, errorCb]{ handleDnsResult(handle, successCb, errorCb); });
    mDns->lookup();
}

QString IdentityResolver::getDnsLookupName(const QString& handle) const
{
    return QString("_atproto.%1").arg(handle);
}

void IdentityResolver::handleDnsResult(const QString& handle, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    if (!mDns)
    {
        qWarning() << "No pending DNS lookup:" << handle;
        return;
    }

    const QString lookupName = getDnsLookupName(handle);

    if (mDns->error() != QDnsLookup::NoError)
    {
        qWarning() << "DNS lookup failed:" << mDns->errorString();
        mDns.reset();
        httpGetDid(handle, successCb, errorCb);
        return;
    }

    qDebug() << "DNS lookup succeeded:" << handle;
    QString did;
    const auto records = mDns->textRecords();

    for (const auto& txt : records)
    {
        if (txt.name() != lookupName)
        {
            qWarning() << "TXT name mismatch:" << txt.name() << "lookup:" << lookupName;
            continue;
        }

        const auto values = txt.values();

        for (const auto& value : values)
        {
            if (!value.startsWith("did="))
            {
                qDebug() << "Skip value:" << value;
                continue;
            }

            const QString didValue = value.sliced(4);

            if (did.isEmpty())
            {
                qDebug() << "Handle:" << handle << "resolved to DID:" << did;
                did = didValue;
            }
            else if (did != didValue)
            {
                qWarning() << "Found multipe DIDs:" << did << didValue;

                if (errorCb)
                    errorCb("Multiple DIDs");

                mDns.reset();
                return;
            }
        }
    }

    if (did.isEmpty())
    {
        qWarning() << "DID not found:" << handle;
        mDns.reset();
        httpGetDid(handle, successCb, errorCb);
        return;
    }

    if (successCb)
        successCb(did);

    mDns.reset();
}

QUrl IdentityResolver::getHttpUrl(const QString& handle) const
{
    return QUrl(QString("https://%1/.well-known/atproto-did").arg(handle));
}

void IdentityResolver::httpGetDid(const QString& handle, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Get DID via HTTP:" << handle;
    QUrl url = getHttpUrl(handle);
    QNetworkRequest request(url);
    QNetworkReply* reply = mNetwork.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, handle, successCb, errorCb]{
        handleHttpResponse(reply, handle, successCb, errorCb);
    });
}

void IdentityResolver::handleHttpResponse(QNetworkReply* reply, const QString& handle, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        const QString& error = reply->errorString();
        qWarning() << "HTTP resolution failed:" << handle << "error:" << error;

        if (errorCb)
            errorCb(error);

        return;
    }

    const auto data = reply->readAll();
    const auto did = QString(data).trimmed();

    if (!ATRegex::isValidDid(did))
    {
        qWarning() << "Invalid DID returned:" << did;

        if (errorCb)
            errorCb("Invalid DID");
    }

    qDebug() << "HTTP resolution succeeded:" << handle << did;

    if (successCb)
        successCb(did);
}

}
