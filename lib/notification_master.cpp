// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "notification_master.h"

namespace ATProto {

namespace {
constexpr char const* DECLARATION_KEY = "self";
}

NotificationMaster::NotificationMaster(Client& client) :
    Presence(),
    mRepoMaster(client)
{
}

void NotificationMaster::getDeclaration(const QString& did, const DeclarationCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Get declatration" << did;
    mRepoMaster.getRecord<AppBskyNotification::Declaration>(
        did, AppBskyNotification::Declaration::TYPE, DECLARATION_KEY, {}, successCb, errorCb);
}

void NotificationMaster::updateDeclaration(const QString& did, const AppBskyNotification::Declaration& declaration,
                                   const SuccessCb& successCb, const ErrorCb& errorCb)
{
    mRepoMaster.updateRecord(did, AppBskyNotification::Declaration::TYPE, DECLARATION_KEY, declaration, successCb, errorCb);
}

}
