// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "chat_master.h"
#include "at_uri.h"

namespace ATProto {

namespace {
constexpr char const* DECLARATION_KEY = "self";
}

ChatMaster::ChatMaster(Client& client) :
    Presence(),
    mClient(client)
{
}

void ChatMaster::getDeclaration(const QString& did, const DeclarationCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Get declatration" << did;
    mClient.getRecord(did, ATUri::COLLECTION_CHAT_ACTOR_DECLARATION, DECLARATION_KEY, {},
        [successCb, errorCb](ComATProtoRepo::Record::Ptr record) {
            qDebug() << "Got declaration:" << record->mValue;

            try {
                auto profile = ChatBskyActor::Declaration::fromJson(record->mValue);

                if (successCb)
                    successCb(std::move(profile));
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

void ChatMaster::updateDeclaration(const QString& did, const ChatBskyActor::Declaration& declaration,
                                   const SuccessCb& successCb, const ErrorCb& errorCb)
{
    mClient.putRecord(did, ATUri::COLLECTION_CHAT_ACTOR_DECLARATION, DECLARATION_KEY, declaration.toJson(), true,
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
