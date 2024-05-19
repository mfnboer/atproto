// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "client.h"
#include "presence.h"
#include "lexicon/chat_bsky_actor.h"

namespace ATProto {

class ChatMaster : public Presence
{
public:
    using DeclarationCb = std::function<void(ChatBskyActor::Declaration::Ptr)>;
    using SuccessCb = Client::SuccessCb;
    using ErrorCb = Client::ErrorCb;

    explicit ChatMaster(Client& client);

    void getDeclaration(const QString& did, const DeclarationCb& successCb, const ErrorCb& errorCb);
    void updateDeclaration(const QString& did, const ChatBskyActor::Declaration& declaration,
                           const SuccessCb& successCb, const ErrorCb& errorCb);

private:
    Client& mClient;
};

}
