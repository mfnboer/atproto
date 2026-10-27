// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "repo_master.h"

namespace ATProto {

RepoMaster::RepoMaster(Client& client) :
    Presence(),
    mClient(client)
{
}

void RepoMaster::deleteRecord(const QString& repo, const QString& collection, const QString& rkey,
                              const SuccessCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Delete record:" << repo << "collection:" << collection << "rkey:" << rkey;
    mClient.deleteRecord(repo, collection, rkey, successCb, errorCb);
}

bool RepoMaster::deleteRecordByUri(const QString& repo, const QString& collection, const QString& uri,
                                   const SuccessCb& successCb, const ErrorCb& errorCb)
{
    const ATUri atUri(uri);

    if (!atUri.isValid())
    {
        qWarning() << "Invalid uri:" << uri;
        return false;
    }

    deleteRecord(repo, collection, atUri.getRkey(), successCb, errorCb);
    return true;
}

ComATProtoRepo::StrongRef::SharedPtr RepoMaster::makeStrongRef(const QString& uri, const QString& cid)
{
    auto ref = std::make_shared<ComATProtoRepo::StrongRef>();
    ref->mUri = uri;
    ref->mCid = cid;
    return ref;
}

bool RepoMaster::addDeleteByUri(ComATProtoRepo::ApplyWritesList& writes, const QString& uri)
{
    const ATUri atUri(uri);

    if (!atUri.isValid())
    {
        qWarning() << "Invalid uri:" << uri;
        return false;
    }

    auto remove = std::make_shared<ComATProtoRepo::ApplyWritesDelete>();
    remove->mCollection = atUri.getCollection();
    remove->mRKey = atUri.getRkey();
    writes.push_back(std::move(remove));
    return true;
}

}
