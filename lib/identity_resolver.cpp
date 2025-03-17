// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "identity_resolver.h"
#include "at_regex.h"
#include "xjson.h"
#include <QJsonDocument>
#include <QJsonObject>

namespace ATProto {

// DNS over HTTP
// Alternative: https://cloudflare-dns.com/dns-query
// Cloudflare JSON values seems to have extra quotes and needs "accept: application/dns-json"
constexpr char const* DOH = "https://dns.google/resolve";

IdentityResolver::IdentityResolver()
{
    mNetwork.setAutoDeleteReplies(true);
    mNetwork.setTransferTimeout(10000);
}

void IdentityResolver::resolveHandle(const QString& handle, const SuccessCb& successCb, const ErrorCb& errorCb)
{
#ifdef Q_OS_ANDROID
    resolveHandleDoh(handle, successCb, errorCb);
#else
    resolveHandleQDns(handle, successCb, errorCb);
#endif
}

// QDnsLookup is not supported on Android
void IdentityResolver::resolveHandleQDns(const QString& handle, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Resolve handle:" << handle;
    const QString lookupName = getDnsLookupName(handle);
    mDns = std::make_unique<QDnsLookup>(QDnsLookup::TXT, lookupName);
    connect(mDns.get(), &QDnsLookup::finished, this, [this, handle, successCb, errorCb]{ handleQDnsResult(handle, successCb, errorCb); });
    mDns->lookup();
}

void IdentityResolver::resolveHandleDoh(const QString& handle, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Resolve handle via DOH:" << handle;
    QUrl url = getDohUrl(handle);
    QNetworkRequest request(url);
    QNetworkReply* reply = mNetwork.get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply, handle, successCb, errorCb]{
        handleDohResponse(reply, handle, successCb, errorCb);
    });
}

QString IdentityResolver::getDnsLookupName(const QString& handle) const
{
    return QString("_atproto.%1").arg(handle);
}

QUrl IdentityResolver::getDohUrl(const QString& handle) const
{
    const QString name = getDnsLookupName(handle);
    return QUrl(QString("%1?name=%2&type=TXT").arg(DOH, name));
}

void IdentityResolver::handleQDnsResult(const QString& handle, const SuccessCb& successCb, const ErrorCb& errorCb)
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
                did = didValue;
                qDebug() << "Handle:" << handle << "resolved to DID:" << did;
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

void IdentityResolver::handleDohResponse(QNetworkReply* reply, const QString& handle, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        const QString& error = reply->errorString();
        qWarning() << "DOH resolution failed:" << handle << "error:" << error;
        httpGetDid(handle, successCb, errorCb);
        return;
    }

    qDebug() << "DOH lookup succeeded:" << handle;
    const auto data = reply->readAll();
    const auto jsonDoc = QJsonDocument::fromJson(data);

    if (jsonDoc.isNull() || !jsonDoc.isObject())
    {
        qWarning() << "Invalid JSON:" << data;

        if (errorCb)
            errorCb(QString("Invalid response from: ") + DOH);

        return;
    }

    const auto json = jsonDoc.object();
    const XJsonObject xjson(json);
    const auto error = xjson.getOptionalString("error");

    if (error)
    {
        qWarning() << "DOH error:" << handle << "error:" << *error;

        if (errorCb)
            errorCb(QString("Error from: ") + DOH + " " + *error);

        return;
    }

    const auto status = xjson.getOptionalInt("Status");

    if (status != 0)
    {
        qWarning() << "DNS error:" << handle << "status:" << *status;

        if (errorCb)
            errorCb(QString("DNS status: %1 from: %2").arg(*status).arg(DOH));
    }

    const auto answerArray = xjson.getOptionalArray("Answer");

    if (!answerArray || answerArray->empty())
    {
        qDebug() << "No TXT record for:" << handle;
        httpGetDid(handle, successCb, errorCb);
        return;
    }

    const QString lookupName = getDnsLookupName(handle);
    const QString lookupFQDN = lookupName + '.';
    QString did;

    for (const auto& answer : *answerArray)
    {
        const auto jsonRecord = answer.toObject();
        const XJsonObject xjsonRecord(jsonRecord);
        const auto name = xjsonRecord.getOptionalString("name");

        if (!name)
        {
            qWarning() << "Name missing, handle:" << handle;
            continue;
        }

        if (*name != lookupName && *name != lookupFQDN)
        {
            qWarning() << "Unexpected name:" << *name << "handle:" << handle;
            continue;
        }

        const auto value = xjsonRecord.getOptionalString("data");

        if (!value)
        {
            qWarning() << "Value missing, handle:" << handle;
            continue;
        }

        if (!value->startsWith("did="))
        {
            qDebug() << "Skip value:" << *value;
            continue;
        }

        const QString didValue = value->sliced(4);

        if (did.isEmpty())
        {
            did = didValue;
            qDebug() << "Handle:" << handle << "resolved to DID:" << did;
        }
        else if (did != didValue)
        {
            qWarning() << "Found multipe DIDs:" << did << didValue;

            if (errorCb)
                errorCb(QString("Multiple DIDs: ") + DOH);

            return;
        }
    }

    if (did.isEmpty())
    {
        qWarning() << "DID not found:" << handle;
        httpGetDid(handle, successCb, errorCb);
        return;
    }

    if (successCb)
        successCb(did);
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
