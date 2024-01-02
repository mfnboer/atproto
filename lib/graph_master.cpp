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
                         const RecordSuccessCb& successCb, const Client::ErrorCb& errorCb)
{
    createRecord<AppBskyGraph::Follow>(did, successCb, errorCb);
}

void GraphMaster::block(const QString& did,
                        const RecordSuccessCb& successCb, const Client::ErrorCb& errorCb)
{
    createRecord<AppBskyGraph::Block>(did, successCb, errorCb);
}

void GraphMaster::listBlock(const QString& listUri,
                            const RecordSuccessCb& successCb, const Client::ErrorCb& errorCb)
{
    createRecord<AppBskyGraph::ListBlock>(listUri, successCb, errorCb);
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

template<class RecordType>
void GraphMaster::createRecord(const QString& subject, const RecordSuccessCb& successCb, const Client::ErrorCb& errorCb)
{
    RecordType record;
    record.mSubject = subject;
    record.mCreatedAt = QDateTime::currentDateTimeUtc();

    const auto recordJson = record.toJson();
    const QString& repo = mClient.getSession()->mDid;
    const QString collection = recordJson["$type"].toString();

    mClient.createRecord(repo, collection, {}, recordJson,
        [successCb](auto strongRef){
            if (successCb)
                successCb(strongRef->mUri, strongRef->mCid);
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

}
