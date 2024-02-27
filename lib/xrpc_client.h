// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "lexicon/com_atproto_server.h"
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace Xrpc {

class Client : public QObject
{
public:
    using ErrorCb = std::function<void(const QString& err, const QJsonDocument& json)>;
    using SuccessJsonCb = std::function<void(const QJsonDocument& json)>;
    using SuccessBytesCb = std::function<void(const QByteArray& bytes, const QString& contentType)>;
    using Params = QList<QPair<QString, QString>>;

    explicit Client(const QString& host);
    ~Client();

    const QString& getHost() const { return mHost; }
    void setPDS(const QString& pds);
    void setPDSFromSession(const ATProto::ComATProtoServer::Session& session);

    void post(const QString& service, const QJsonDocument& json,
              const SuccessJsonCb& successCb, const ErrorCb& errorCb, const QString& accessJwt = {});
    void post(const QString& service, const QByteArray& data, const QString& mimeType,
              const SuccessJsonCb& successCb, const ErrorCb& errorCb, const QString& accessJwt);
    void get(const QString& service, const Params& params,
             const SuccessJsonCb& successCb, const ErrorCb& errorCb, const QString& accessJwt = {});
    void get(const QString& service, const Params& params,
             const SuccessBytesCb& successCb, const ErrorCb& errorCb, const QString& accessJwt = {});

private:
    struct Request
    {
        bool mIsPost = false;
        QNetworkRequest mXrpcRequest;
        QByteArray mData;
        int mResendCount = 0;
    };

    QUrl buildUrl(const QString& service) const;
    QUrl buildUrl(const QString& service, const Params& params) const;
    void setAuthorization(QNetworkRequest& request, const QString& accessJwt) const;

    template<typename Callback>
    void getImpl(const QString& service, const Params& params,
                 const Callback& successCb, const ErrorCb& errorCb, const QString& accessJwt);

    template<typename Callback>
    void replyFinished(const Request& request, QNetworkReply* reply,
                       const Callback& successCb, const ErrorCb& errorCb,
                       std::shared_ptr<bool> errorHandled);

    template<typename Callback>
    void networkError(const Request& request, QNetworkReply* reply, QNetworkReply::NetworkError errorCode,
                      const Callback& successCb, const ErrorCb& errorCb,
                      std::shared_ptr<bool> errorHandled);

    void sslErrors(QNetworkReply* reply, const QList<QSslError>& errors, const ErrorCb& errorCb, std::shared_ptr<bool> errorHandled);

    template<typename Callback>
    void sendRequest(const Request& request, const Callback& successCb, const ErrorCb& errorCb);

    template<typename Callback>
    bool resendRequest(Request request, const Callback& successCb, const ErrorCb& errorCb);

    bool mustResend(QNetworkReply::NetworkError error) const;

    QString mHost; // first point of contact, e.g. bsky.social
    QString mPDS;

    // TODO: can we make it non-static?
    // Making this static as on Android it sometimes causes a crash when you destroy
    // this object. It looks like Android still wants to send a signal to it after
    // a network failure. Not sure: the logs do not give enough info on that.
    QNetworkAccessManager sNetwork;
};

}
