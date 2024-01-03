// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "graph_master.h"
#include "at_uri.h"
#include "lexicon/app_bsky_graph.h"

namespace ATProto {

GraphMaster::GraphMaster(Client & client) :
    Presence(),
    mClient(client),
    mRichTextMaster(client)
{
}

void GraphMaster::follow(const QString& did,
                         const RecordSuccessCb& successCb, const ErrorCb& errorCb)
{
    createRecord<AppBskyGraph::Follow>(did, successCb, errorCb);
}

void GraphMaster::block(const QString& did,
                        const RecordSuccessCb& successCb, const ErrorCb& errorCb)
{
    createRecord<AppBskyGraph::Block>(did, successCb, errorCb);
}

void GraphMaster::listBlock(const QString& listUri,
                            const RecordSuccessCb& successCb, const ErrorCb& errorCb)
{
    createRecord<AppBskyGraph::ListBlock>(listUri, successCb, errorCb);
}

void GraphMaster::undo(const QString& uri,
                       const Client::SuccessCb& successCb, const ErrorCb& errorCb)
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

void GraphMaster::createList(AppBskyGraph::ListPurpose purpose, const QString& name,
                const QString& description, Blob::Ptr avatar,
                const CreateListSuccessCb& successCb, const ErrorCb& errorCb)
{
    auto list = std::make_shared<AppBskyGraph::List>();
    list->mPurpose = purpose;
    list->mName = name;
    list->mAvatar = std::move(avatar);
    list->mCreatedAt = QDateTime::currentDateTimeUtc();
    auto facets = RichTextMaster::parseFacets(description);

    mRichTextMaster.resolveFacets(description, facets, 0,
        [this, presence=getPresence(), list, successCb, errorCb](const QString& richText, AppBskyRichtext::FacetList resolvedFacets){
            if (!presence)
                return;

            list->mDescription = richText;
            list->mDescriptionFacets = std::move(resolvedFacets);
            createList(*list, successCb, errorCb);
        });
}

void GraphMaster::createList(const AppBskyGraph::List& list, const CreateListSuccessCb& successCb, const ErrorCb& errorCb)
{
    const auto listJson = list.toJson();
    qDebug() << "Create list:" << listJson;
    const QString& repo = mClient.getSession()->mDid;
    const QString collection = listJson["$type"].toString();

    mClient.createRecord(repo, collection, {}, listJson,
        [successCb](auto strongRef){
            if (successCb)
                successCb(strongRef->mUri, strongRef->mCid);
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

void GraphMaster::updateList(const AppBskyGraph::List& list, const QString& rkey, const SuccessCb& successCb, const ErrorCb& errorCb)
{
    const auto listJson = list.toJson();
    qDebug() << "Update list:" << listJson;
    const QString& repo = mClient.getSession()->mDid;
    const QString collection = listJson["$type"].toString();

    mClient.putRecord(repo, collection, rkey, listJson,
        [successCb](auto){
            if (successCb)
                successCb();
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

template<class RecordType>
void GraphMaster::createRecord(const QString& subject, const RecordSuccessCb& successCb, const ErrorCb& errorCb)
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
