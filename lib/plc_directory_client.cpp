// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "plc_directory_client.h"
#include "lexicon.h"
#include <QJsonObject>

namespace ATProto {

constexpr int MAX_RESEND = 4;

PlcDirectoryClient::PlcDirectoryClient(QNetworkAccessManager* network, const QString host, QObject* parent) :
    QObject(parent),
    mNetwork(network),
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
    request.mPlcRequest = QNetworkRequest(url);

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

                qDebug() << "Resolved PDS for:" << did << *didDoc->mATProtoPDS;
                mPdsCache.insert(did, new QString(*didDoc->mATProtoPDS));

                if (successCb)
                    successCb(*didDoc->mATProtoPDS);
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
    request.mPlcRequest = QNetworkRequest(url);

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

// TODO: refactor code common with XrpcClient
void PlcDirectoryClient::sendRequest(const Request& request, const SuccessJsonCb& successCb, const ErrorCb& errorCb)
{
    QNetworkReply* reply;
    reply = mNetwork->get(request.mPlcRequest);

    // In case of an error multiple callbacks may fire. First errorOcccured() and then probably finished()
    // The latter call is not guaranteed however. We must only call errorCb once!
    auto errorHandled = std::make_shared<bool>(false);

    connect(reply, &QNetworkReply::finished, this,
            [this, request, reply, successCb, errorCb, errorHandled]{ replyFinished(request, reply, successCb, errorCb, errorHandled); });
    connect(reply, &QNetworkReply::errorOccurred, this,
            [this, request, reply, successCb, errorCb, errorHandled](auto errorCode){ this->networkError(request, reply, errorCode, successCb, errorCb, errorHandled); });
    connect(reply, &QNetworkReply::sslErrors, this,
            [this, reply, errorCb, errorHandled](const QList<QSslError>& errors){ sslErrors(reply, errors, errorCb, errorHandled); });
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
    qInfo() << "Network error:" << errorCode << errorMsg;

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

void PlcDirectoryClient::sslErrors(QNetworkReply* reply, const QList<QSslError>& errors, const ErrorCb& errorCb, std::shared_ptr<bool> errorHandled)
{
    Q_ASSERT(reply);
    qWarning() << "SSL errors:" << errors;

    if (!*errorHandled)
    {
        *errorHandled = true;
        QString msg = "SSL error";

        if (!errors.empty())
            msg.append(": ").append(errors.front().errorString());

        errorCb(reply->error(), msg);
    }
    else
    {
        qDebug() << "Error already handled";
    }
}

bool PlcDirectoryClient::resendRequest(Request request, const SuccessJsonCb& successCb, const ErrorCb& errorCb)
{
    if (request.mResendCount >= MAX_RESEND)
    {
        qWarning() << "Maximum resends reached:" << request.mPlcRequest.url();
        return false;
    }

    ++request.mResendCount;
    qDebug() << "Resend:" << request.mPlcRequest.url() << "count:" << request.mResendCount;
    sendRequest(request, successCb, errorCb);
    return true;
}

bool PlcDirectoryClient::mustResend(QNetworkReply::NetworkError error) const
{
    switch (error)
    {
    case QNetworkReply::NoError: // Unknown error seems to happen sometimes since Qt6.9.2
        qWarning() << "Retry PLC on unknown error";
    case QNetworkReply::ContentReSendError:
    case QNetworkReply::OperationCanceledError: // Timeout
    case QNetworkReply::RemoteHostClosedError:
        return true;
    default:
        break;
    }

    return false;
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
