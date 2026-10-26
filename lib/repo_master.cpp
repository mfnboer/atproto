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


}
