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

    mClient.createRecord(repo, collection, followJson,
        [this, successCb](auto strongRef){
            if (successCb)
                successCb(strongRef->mUri, strongRef->mCid);
        },
        [this, errorCb](const QString& error) {
            if (errorCb)
                errorCb(error);
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
        [errorCb](const QString& err) {
            if (errorCb)
                errorCb(err);
        });
}

}
