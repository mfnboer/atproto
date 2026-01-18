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
    mClient.deleteRecord(repo, collection, rkey,
        [successCb]{
            if (successCb)
                successCb();
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

}
