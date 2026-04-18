// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "plc_directory_client.h"
#include "lexicon.h"
#include <QJsonObject>

namespace ATProto {

static QString normalizeHost(const QString& host)
{
    QString normalized = host;

    if (!normalized.startsWith("http"))
        normalized = "https://" + normalized;

    if (normalized.endsWith('/'))
        normalized.chop(1);

    return normalized;
}

PlcDirectoryClient::PlcDirectoryClient(QNetworkAccessManager* network, const QString host, QObject* parent) :
    NetworkClient<PlcRequest, PlcSuccessJsonCb, PlcErrorCb>(network, parent),
    mHost(host),
    mFirstAppearanceCache(100),
    mPdsCache(100)
{
    Q_ASSERT(mNetwork);
    Q_ASSERT(mNetwork->autoDeleteReplies());
}

void PlcDirectoryClient::getPds(const QString& did, const PdsSuccessCb& successCb, const ErrorCb& errorCb)
{
    if (mPdsCache.contains(did))
    {
        const QString* pds = mPdsCache[did];
        qDebug() << "Got PDS from cache:" << did << *pds;

        if (successCb)
            successCb(*pds);

        return;
    }

    Request request;
    QUrl url(QString("https://%1/%2").arg(mHost, did));
    request.mNetworkRequest = QNetworkRequest(url);
    setUserAgentHeader(request.mNetworkRequest);

    sendRequest(request,
        [this, presence=getPresence(), did, successCb, errorCb](const QJsonDocument& reply) {
            if (!presence)
                return;

            qDebug() << "getPds:" << reply;
            try {
                auto didDoc = DidDocument::fromJson(reply.object());

                if (!didDoc->mATProtoPDS)
                {
                    qWarning() << "Cannot resolve PDS for:" << did;

                    if (errorCb)
                        errorCb(404, "Cannot resolve PDS");

                    return;
                }

                const QString pds = normalizeHost(*didDoc->mATProtoPDS);
                qDebug() << "Resolved PDS for:" << did << *didDoc->mATProtoPDS << "normalized:" << pds;
                mPdsCache.insert(did, new QString(pds));

                if (successCb)
                    successCb(pds);
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        [errorCb](int errorCode, const QString& errorMsg) {
            qWarning() << errorCode << "-" << errorMsg;
            if (errorCb)
                errorCb(errorCode, errorMsg);
        });
}

void PlcDirectoryClient::getAuditLog(const QString& did, const AuditLogSuccessCb& successCb, const ErrorCb& errorCb)
{
    Request request;
    QUrl url(QString("https://%1/%2/log/audit").arg(mHost, did));
    request.mNetworkRequest = QNetworkRequest(url);
    setUserAgentHeader(request.mNetworkRequest);

    sendRequest(request,
        [this, presence=getPresence(), successCb, errorCb](const QJsonDocument& reply) {
            if (!presence)
                return;

            qDebug() << "getAuditLog:" << reply;

            try {
                auto auditLog = PlcAuditLog::fromJson(reply);

                if (successCb)
                    successCb(std::move(auditLog));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        [errorCb](int errorCode, const QString& errorMsg) {
            qDebug() << errorCode << "-" << errorMsg;
            if (errorCb)
                errorCb(errorCode, errorMsg);
        });
}

void PlcDirectoryClient::getFirstAppearance(const QString& did, const FirstAppearanceSuccessCb& successCb, const ErrorCb& errorCb)
{
    if (mFirstAppearanceCache.contains(did))
    {
        const QDateTime* appearance = mFirstAppearanceCache[did];
        qDebug() << "First appearance from cache:" << *appearance;

        if (successCb)
            successCb(*appearance);

        return;
    }

    getAuditLog(did,
        [this, presence=getPresence(), did, successCb, errorCb](PlcAuditLog::SharedPtr auditLog) {
            if (!presence)
                return;

            if (auditLog->mEntries.empty())
            {
                qWarning() << "Empty audit log";

                if (errorCb)
                    errorCb(-1, "Empty audit log");
            }

            const QDateTime appearance = auditLog->mEntries.front()->mCreatedAt;
            qDebug() << "getFirstAppearance:" << appearance;
            mFirstAppearanceCache.insert(did, new QDateTime(appearance));

            if (successCb)
                successCb(appearance);
        },
        [errorCb](int errorCode, const QString& errorMsg) {
            qDebug() << errorCode << "-" << errorMsg;
            if (errorCb)
                errorCb(errorCode, errorMsg);
        });
}

void PlcDirectoryClient::invalidatePdsCache(const QString& did)
{
    qDebug() << "Invalidate PDS cache:" << did;
    mPdsCache.remove(did);
}

void PlcDirectoryClient::replyFinished(const Request& request, QNetworkReply* reply,
                                       const SuccessJsonCb& successCb, const ErrorCb& errorCb,
                                       std::shared_ptr<bool> errorHandled)
{
    Q_ASSERT(reply);
    const auto errorCode = reply->error();
    const QString contentType = reply->header(QNetworkRequest::ContentTypeHeader).toString().trimmed();
    qDebug() << "Reply:" << errorCode << "content:" << contentType;
    const auto data = reply->readAll();

    if (errorCode == QNetworkReply::NoError)
    {
        const QJsonDocument json(QJsonDocument::fromJson(data));
        successCb(json);
    }
    else if (!*errorHandled)
    {
        *errorHandled = true;

        if (mustResend(errorCode))
        {
            if (resendRequest(request, successCb, errorCb))
                return;
        }

        const QJsonDocument json(QJsonDocument::fromJson(data));
        invokeErrorCb(json, reply, errorCode, errorCb);
    }
    else
    {
        qDebug() << "Error already handled";
    }
}

void PlcDirectoryClient::networkError(const Request& request, QNetworkReply* reply, QNetworkReply::NetworkError errorCode,
                                      const SuccessJsonCb& successCb, const ErrorCb& errorCb,
                                      std::shared_ptr<bool> errorHandled)
{
    Q_ASSERT(reply);
    const auto errorMsg = reply->errorString();
    qWarning() << "Network error:" << errorCode << errorMsg;

    if (!*errorHandled)
    {
        *errorHandled = true;

        if (errorCode == QNetworkReply::OperationCanceledError)
            reply->disconnect();

        if (mustResend(errorCode))
        {
            qDebug() << "Try resend on error:" << errorCode << errorMsg;

            if (resendRequest(request, successCb, errorCb))
                return;
        }

        if (errorCode == QNetworkReply::OperationCanceledError)
        {
            errorCb(errorCode, errorMsg);
            return;
        }

        const auto data = reply->readAll();
        const QJsonDocument json(QJsonDocument::fromJson(data));
        invokeErrorCb(json, reply, errorCode, errorCb);
    }
    else
    {
        qDebug() << "Error already handled";
    }
}

void PlcDirectoryClient::invalidJsonError(InvalidJsonException& e, const ErrorCb& cb)
{
    qWarning() << e.msg();
    if (cb)
        cb(-1, e.msg());
}

void PlcDirectoryClient::invokeErrorCb(const QJsonDocument& jsonDoc, QNetworkReply* reply, QNetworkReply::NetworkError errorCode, const ErrorCb& errorCb)
{
    try {
        auto plcError = PlcError::fromJson(jsonDoc.object());
        errorCb(errorCode, plcError->mMessage.value_or(reply->errorString()));
    } catch (InvalidJsonException& e) {
        qWarning() << e.msg();
        errorCb(errorCode, reply->errorString());
    }
}

}
