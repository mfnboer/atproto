// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "client.h"

/**
 * Functions to change the social graph, e.g. follow, block, ...
 */
namespace ATProto {

class GraphMaster
{
public:
    using RecordSuccessCb = std::function<void(const QString& uri, const QString& cid)>;

    explicit GraphMaster(Client& client);

    void follow(const QString& did,
                const RecordSuccessCb& successCb, const Client::ErrorCb& errorCb);
    void block(const QString& did,
               const RecordSuccessCb& successCb, const Client::ErrorCb& errorCb);
    void listBlock(const QString& listUri,
                   const RecordSuccessCb& successCb, const Client::ErrorCb& errorCb);
    void undo(const QString& uri,
              const Client::SuccessCb& successCb, const Client::ErrorCb& errorCb);

private:
    template<class RecordType>
    void createRecord(const QString& subject, const RecordSuccessCb& successCb, const Client::ErrorCb& errorCb);

    Client& mClient;
    QObject mPresence;
};

}
