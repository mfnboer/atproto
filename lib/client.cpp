// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "client.h"
#include "xjson.h"

namespace ATProto
{

Client::Client(std::unique_ptr<Xrpc::Client>&& xrpc) :
    mXrpc(std::move(xrpc))
{}

void Client::createSession(const QString& user, const QString& pwd,
                           const createSessionSuccessCb& successCb, const ErrorCb& errorCb)
{
    mSession = nullptr;
    QJsonObject root;
    root.insert("identifier", user);
    root.insert("password", pwd);
    QJsonDocument json(root);

    mXrpc->post("com.atproto.server.createSession", json,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "Session created:" << reply;
            try {
                mSession = std::move(ComATProtoServer::Session::fromJson(reply));
                if (successCb)
                    successCb();
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb));
}

void Client::getProfile(const QString& user, const getProfileSuccessCb& successCb, const ErrorCb& errorCb)
{
    mXrpc->get("app.bsky.actor.getProfile", {{"actor", user}},
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getProfile:" << reply;
            try {
                auto profile = AppBskyActor::ProfileViewDetailed::fromJson(reply);
                if (successCb)
                    successCb(std::move(profile));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

void Client::getAuthorFeed(const QString& user, std::optional<int> limit, const std::optional<QString>& cursor,
                   const getAuthorFeedSuccessCb& successCb, const ErrorCb& errorCb)
{
    Xrpc::Client::Params params{{"actor", user}};

    if (limit)
    {
        if (*limit < 1 || * limit > 100)
            throw InvalidRequest(QString("Invalid limit value %1").arg(*limit));

        params.append({"limit", QString::number(*limit)});
    }

    if (cursor)
        params.append({"cursor", *cursor});

    mXrpc->get("app.bsky.feed.getAuthorFeed", params,
        [this, successCb, errorCb](const QJsonDocument& reply){
            qDebug() << "getAuthorFeed:" << reply;
            try {
                auto feed = AppBskyFeed::AuthorFeed::fromJson(reply);
                if (successCb)
                    successCb(std::move(feed));
            } catch (InvalidJsonException& e) {
                invalidJsonError(e, errorCb);
            }
        },
        failure(errorCb),
        authToken());
}

const QString& Client::authToken() const
{
    static const QString NO_TOKEN;
    return mSession ? mSession->mAccessJwt : NO_TOKEN;
}

Xrpc::Client::ErrorCb Client::failure(const ErrorCb& cb)
{
    return [this, cb](const QString& err, const QJsonDocument& reply){
            requestFailed(err, reply, cb);
        };
}

void Client::invalidJsonError(InvalidJsonException& e, const ErrorCb& cb)
{
    qWarning() << e.msg();
    if (cb)
        cb(e.msg());
}

void Client::requestFailed(const QString& err, const QJsonDocument& json, const ErrorCb& errorCb)
{
    qDebug() << "Request failed:" << err;
    qDebug() << json;

    // HTTP API (XRPC): error responses must contain json body with error and message fields.
    try {
        XJsonObject xjson(json.object());
        const QString error = xjson.getRequiredString("error");
        const QString msg = xjson.getRequiredString("message");

        if (errorCb)
            errorCb(QString("%1 - %2").arg(error, msg));
    } catch (InvalidJsonException& e) {
        qWarning() << e.msg();

        if (errorCb)
            errorCb(err);
    }
}

}
