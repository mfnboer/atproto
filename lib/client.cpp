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
    mSessionCreated = false;
    QJsonObject root;
    root.insert("identifier", user);
    root.insert("password", pwd);
    QJsonDocument json(root);

    mXrpc->post("com.atproto.server.createSession", json,
        [this, successCb, errorCb](const QJsonDocument& reply){
            sessionCreated(reply, successCb, errorCb);
        },
        [this, errorCb](const QString& err, const QJsonDocument& reply){
            requestFailed(err, reply, errorCb);
        });
}

void Client::sessionCreated(const QJsonDocument& json, const createSessionSuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Session created:" << json;

    if (!json.isObject())
    {
        qWarning() << "JSON missing from createSession reply";
        if (errorCb)
            errorCb("JSON missing from createSession reply");
        return;
    }

    try {
        // TODO: define lexicon struct
        XJsonObject root(json.object());
        mUserHandle = root.getRequiredString("handle");
        mUserDid = root.getRequiredString("did");
        mAccessJwt = root.getRequiredString("accessJwt");
        mRefreshJwt = root.getRequiredString("refreshJwt");
    } catch (InvalidJsonException& e) {
        qWarning() << e.msg();
        if (errorCb)
            errorCb(e.msg());
        return;
    }

    qDebug() << "Handle:" << mUserHandle;
    mSessionCreated = true;

    if (successCb)
        successCb();
}

void Client::getProfile(const QString& user, const getProfileSuccessCb& successCb, const ErrorCb& errorCb)
{
    mXrpc->get("app.bsky.actor.getProfile", {{"actor", user}},
        [this, successCb, errorCb](const QJsonDocument& reply){
            getProfileSuccess(reply, successCb, errorCb);
        },
        [this, errorCb](const QString& err, const QJsonDocument& reply){
            requestFailed(err, reply, errorCb);
        },
        mAccessJwt);
}

void Client::getProfileSuccess(const QJsonDocument& json, const getProfileSuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "getProfile:" << json;

    try {
        const AppBskyActor::ProfileViewDetailed profile = AppBskyActor::fromJson(json);
        if (successCb)
            successCb(profile);
    } catch (InvalidJsonException& e) {
        qWarning() << e.msg();
        if (errorCb)
            errorCb(e.msg());
    }
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
