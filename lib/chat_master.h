// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "client.h"
#include "presence.h"
#include "rich_text_master.h"
#include "lexicon/chat_bsky_actor.h"

namespace ATProto {

class ChatMaster : public Presence
{
public:
    using MessageCreatedCb = std::function<void(ChatBskyConvo::MessageInput::SharedPtr)>;
    using DeclarationCb = std::function<void(ChatBskyActor::Declaration::SharedPtr)>;
    using SuccessCb = Client::SuccessCb;
    using ErrorCb = Client::ErrorCb;

    explicit ChatMaster(Client& client);

    void getDeclaration(const QString& did, const DeclarationCb& successCb, const ErrorCb& errorCb);
    void updateDeclaration(const QString& did, const ChatBskyActor::Declaration& declaration,
                           const SuccessCb& successCb, const ErrorCb& errorCb);

    void createMessage(const QString& text, const MessageCreatedCb& cb);
    static void addQuoteToMessage(ChatBskyConvo::MessageInput& message, const QString& quoteUri, const QString& quoteCid);

private:
    Client& mClient;
    RichTextMaster mRichTextMaster;
};

}
