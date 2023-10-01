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
    using FollowSuccessCb = std::function<void(const QString& uri, const QString& cid)>;

    explicit GraphMaster(Client& client);

    void follow(const QString& did,
                const FollowSuccessCb& successCb, const Client::ErrorCb& errorCb);
    void undo(const QString& uri,
              const Client::SuccessCb& successCb, const Client::ErrorCb& errorCb);

private:
    Client& mClient;
    QObject mPresence;
};

}
