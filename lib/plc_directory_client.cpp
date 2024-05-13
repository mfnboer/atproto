// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "plc_directory_client.h"
#include <QJsonObject>

namespace ATProto {

constexpr int MAX_RESEND = 4;

PlcDirectoryClient::PlcDirectoryClient(const QString host) :
    mHost(host),
    mFirstAppearanceCache(100)
{
    mNetwork.setAutoDeleteReplies(true);
    mNetwork.setTransferTimeout(15000);
}

void PlcDirectoryClient::getAuditLog(const QString& did, const AuditLogSuccessCb& successCb, const ErrorCb& errorCb)
{
    Request request;
    QUrl url(QString("https://%1/%2/log/audit").arg(mHost, did));
    request.mPlcRequest = QNetworkRequest(url);
    sendRequest(request,
        [this, successCb, errorCb](const QJsonDocument& reply) {
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
            emit successCb(*appearance);

        return;
    }

    getAuditLog(did,
        [this, did, successCb, errorCb](PlcAuditLog::Ptr auditLog) {
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

// TODO: refactor code common with XrpcClient
void PlcDirectoryClient::sendRequest(const Request& request, const SuccessJsonCb& successCb, const ErrorCb& errorCb)
{
    QNetworkReply* reply;
    reply = mNetwork.get(request.mPlcRequest);

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

    if (errorCode == QNetworkReply::OperationCanceledError)
    {
        reply->disconnect();
        errorCb(errorCode, errorMsg);
        return;
    }

    const auto data = reply->readAll();

    if (!*errorHandled)
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
    case QNetworkReply::ContentReSendError:
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
