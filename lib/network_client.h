// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "presence.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace ATProto {

template<typename RequestType, typename RequestSuccessCb, typename RequestErrorCb>
class NetworkClient : public QObject, public Presence
{
public:
    static bool isSafeUrl(const QUrl url)
    {
        if (url.scheme() != "https" ||
            url.host().isEmpty() ||
            !url.userName().isEmpty() ||
            !url.password().isEmpty() ||
            url.port() > -1)
        {
            qWarning() << "Unsafe URL:" << url;
            return false;
        }

        const auto segments = url.host().split('.');

        if (segments.length() < 2)
        {
            qWarning() << "Not enough segments in host:" << url;
            return false;
        }

        static const std::unordered_set<QString> FORBIDDEN_DOMAINS = {"local", "arpa", "internal", "localhost"};

        if (FORBIDDEN_DOMAINS.contains(segments.last()))
        {
            qWarning() << "Forbidden domain in URL:" << url;
            return false;
        }

        return true;
    }

    explicit NetworkClient(QNetworkAccessManager* network, QObject* parent = nullptr) :
        QObject(parent),
        mNetwork(network)
    {
    }

    virtual ~NetworkClient() = default;

    void setUserAgent(const QString& userAgent) { mUserAgent = userAgent; }

protected:
    static constexpr int MAX_RESEND = 4;

    virtual void replyFinished(const RequestType& request, QNetworkReply* reply,
                       const RequestSuccessCb& successCb, const RequestErrorCb& errorCb,
                       std::shared_ptr<bool> errorHandled) = 0;

    virtual void networkError(const RequestType& request, QNetworkReply* reply, QNetworkReply::NetworkError errorCode,
                      const RequestSuccessCb& successCb, const RequestErrorCb& errorCb,
                      std::shared_ptr<bool> errorHandled) = 0;

    void sslErrors(QNetworkReply* reply, const QList<QSslError>& errors, const RequestErrorCb& errorCb, std::shared_ptr<bool> errorHandled)
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

    virtual void sendRequest(const RequestType& request, const RequestSuccessCb& successCb, const RequestErrorCb& errorCb)
    {
        QNetworkReply* reply;

        if (request.mIsPost)
            reply = mNetwork->post(request.mNetworkRequest, request.mPostData);
        else
            reply = mNetwork->get(request.mNetworkRequest);

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

    bool mustResend(QNetworkReply::NetworkError error) const
    {
        switch (error)
        {
        case QNetworkReply::NoError: // Unknown error seems to happen sometimes since Qt6.9.2
            qWarning() << "Retry on unknown error";
        case QNetworkReply::ContentReSendError:
        case QNetworkReply::OperationCanceledError: // Timeout
        case QNetworkReply::RemoteHostClosedError:
        case QNetworkReply::TimeoutError:
            return true;
        default:
            break;
        }

        return false;
    }

    bool resendRequest(RequestType request, const RequestSuccessCb& successCb, const RequestErrorCb& errorCb)
    {
        if (request.mResendCount >= MAX_RESEND)
        {
            qWarning() << "Maximum resends reached:" << request.mNetworkRequest.url();
            return false;
        }

        ++request.mResendCount;
        qDebug() << "Resend:" << request.mNetworkRequest.url() << "count:" << request.mResendCount;
        sendRequest(request, successCb, errorCb);
        return true;
    }

    void setUserAgentHeader(QNetworkRequest& request) const
    {
        if (!mUserAgent.isEmpty())
            request.setHeader(QNetworkRequest::UserAgentHeader, mUserAgent);
    }

    QNetworkAccessManager* mNetwork;
    QString mUserAgent;
};

}
