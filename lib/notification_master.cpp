// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#include "notification_master.h"

namespace ATProto {

namespace {
constexpr char const* DECLARATION_KEY = "self";
}

NotificationMaster::NotificationMaster(Client& client) :
    Presence(),
    mClient(client)
{
}

void NotificationMaster::getDeclaration(const QString& did, const DeclarationCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Get declatration" << did;
    mClient.getRecord(did, AppBskyNotification::Declaration::TYPE, DECLARATION_KEY, {},
        [successCb, errorCb](ComATProtoRepo::Record::SharedPtr record) {
            qDebug() << "Got declaration:" << record->mValue;

            try {
                auto declaration = AppBskyNotification::Declaration::fromJson(record->mValue);

                if (successCb)
                    successCb(std::move(declaration));
            } catch (InvalidJsonException& e) {
                qWarning() << e.msg();

                if (errorCb)
                    errorCb("InvalidJsonException", e.msg());
            }
        },
        [errorCb](const QString& err, const QString& msg) {
            qDebug() << "Failed to get declaration:" << err << "-" << msg;

            if (errorCb)
                errorCb(err, msg);
        });
}

void NotificationMaster::updateDeclaration(const QString& did, const AppBskyNotification::Declaration& declaration,
                                   const SuccessCb& successCb, const ErrorCb& errorCb)
{
    mClient.putRecord(did, AppBskyNotification::Declaration::TYPE, DECLARATION_KEY, declaration.toJson(), true,
        [successCb](auto){
            if (successCb)
                successCb();
        },
        [errorCb](const QString& error, const QString& msg) {
            if (errorCb)
                errorCb(error, msg);
        });
}

}
