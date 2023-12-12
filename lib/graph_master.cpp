// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "graph_master.h"
#include "at_uri.h"
#include "lexicon/app_bsky_graph.h"

namespace ATProto {

GraphMaster::GraphMaster(Client & client) :
    mClient(client)
{
}

void GraphMaster::follow(const QString& did,
                         const FollowSuccessCb& successCb, const Client::ErrorCb& errorCb)
{
    AppBskyGraph::Follow follow;
    follow.mSubject = did;
    follow.mCreatedAt = QDateTime::currentDateTimeUtc();

    const auto followJson = follow.toJson();
    const QString& repo = mClient.getSession()->mDid;
    const QString collection = followJson["$type"].toString();

    mClient.createRecord(repo, collection, {}, followJson,
        [successCb](auto strongRef){
            if (successCb)
                successCb(strongRef->mUri, strongRef->mCid);
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

void GraphMaster::block(const QString& did,
                        const FollowSuccessCb& successCb, const Client::ErrorCb& errorCb)
{
    AppBskyGraph::Block block;
    block.mSubject = did;
    block.mCreatedAt = QDateTime::currentDateTimeUtc();

    const auto blockJson = block.toJson();
    const QString& repo = mClient.getSession()->mDid;
    const QString collection = blockJson["$type"].toString();

    mClient.createRecord(repo, collection, {}, blockJson,
        [successCb](auto strongRef){
            if (successCb)
                successCb(strongRef->mUri, strongRef->mCid);
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

void GraphMaster::undo(const QString& uri,
                       const Client::SuccessCb& successCb, const Client::ErrorCb& errorCb)
{
    qDebug() << "Undo:" << uri;
    const auto atUri = ATUri::createAtUri(uri, mPresence, errorCb);
    if (!atUri.isValid())
        return;

    mClient.deleteRecord(atUri.getAuthority(), atUri.getCollection(), atUri.getRkey(),
        [successCb]{
            if (successCb)
                successCb();
        },
        [errorCb](const QString& err, const QString& msg) {
            if (errorCb)
                errorCb(err, msg);
        });
}

}
