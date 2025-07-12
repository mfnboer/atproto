// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include "client.h"
#include "presence.h"
#include <lexicon/app_bsky_notification.h>

namespace ATProto {

class NotificationMaster : public Presence
{
public:
    using DeclarationCb = std::function<void(AppBskyNotification::Declaration::SharedPtr)>;
    using SuccessCb = Client::SuccessCb;
    using ErrorCb = Client::ErrorCb;

    explicit NotificationMaster(Client& client);

    void getDeclaration(const QString& did, const DeclarationCb& successCb, const ErrorCb& errorCb);
    void updateDeclaration(const QString& did, const AppBskyNotification::Declaration& declaration,
                           const SuccessCb& successCb, const ErrorCb& errorCb);

private:
    Client& mClient;
};

}
