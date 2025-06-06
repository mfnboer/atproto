// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QThread>

namespace Xrpc {

class NetworkThread : public QThread
{
    Q_OBJECT
public:
    using DataType = std::variant<QByteArray, QIODevice*>;
    using ErrorCb = std::function<void(const QString& err, const QJsonDocument& json)>;
    using SuccessJsonCb = std::function<void(const QJsonDocument& json)>;
    using SuccessBytesCb = std::function<void(const QByteArray& bytes, const QString& contentType)>;
    using CallbackType = std::variant<SuccessJsonCb, SuccessBytesCb>;
    using Params = QList<QPair<QString, QString>>;

    struct Request
    {
        bool mIsPost = false;
        QNetworkRequest mXrpcRequest;
        DataType mData;
        int mResendCount = 0;
    };

    NetworkThread(QObject* parent = nullptr);
    void setPDS(const QString& pds) { mPDS = pds; }
    void setUserAgent(const QString& userAgent) { mUserAgent = userAgent; }
    void sendRequest(const Request& request, const CallbackType& successCb, const ErrorCb& errorCb);
    void postData(const QString& service, const DataType& data, const QString& mimeType, const Params& rawHeaders,
                  const SuccessJsonCb& successCb, const ErrorCb& errorCb, const QString& accessJwt);
    void postJson(const QString& service, const QJsonDocument& json, const Params& rawHeaders,
                  const SuccessJsonCb& successCb, const ErrorCb& errorCb, const QString& accessJwt);
    void get(const QString& service, const Params& params, const Params& rawHeaders,
             const CallbackType& successCb, const ErrorCb& errorCb, const QString& accessJwt);

signals:
    void requestSuccessJson(QJsonDocument json, SuccessJsonCb cb);
    void requestSuccessBytes(QByteArray bytes, SuccessBytesCb cb, QString contentType);
    void requestError(QString error, QJsonDocument json, ErrorCb cb);

protected:
    virtual void run() override;

private:
    QUrl buildUrl(const QString& service) const;
    QUrl buildUrl(const QString& service, const Params& params) const;
    void setUserAgentHeader(QNetworkRequest& request) const;
    void setAuthorization(QNetworkRequest& request, const QString& accessJwt) const;
    void setRawHeaders(QNetworkRequest& request, const Params& params) const;

    bool resendRequest(Request request, const CallbackType& successCb, const ErrorCb& errorCb);
    bool mustResend(QNetworkReply::NetworkError error) const;
    void invokeCallback(CallbackType successCb, QByteArray data, const QString& contentType);
    void replyFinished(const Request& request, QNetworkReply* reply,
                       CallbackType successCb, const ErrorCb& errorCb,
                       std::shared_ptr<bool> errorHandled);
    void networkError(const Request& request, QNetworkReply* reply, QNetworkReply::NetworkError errorCode,
                      const CallbackType& successCb, const ErrorCb& errorCb,
                      std::shared_ptr<bool> errorHandled);
    void sslErrors(QNetworkReply* reply, const QList<QSslError>& errors, const ErrorCb& errorCb, std::shared_ptr<bool> errorHandled);

    struct Task
    {
        Request mRequest;
        CallbackType mCb;
    };

    QNetworkAccessManager* mNetwork;
    QString mPDS;
    QString mUserAgent;
};

}
