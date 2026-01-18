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
    mRichTextMaster(client),
    mRepoMaster(client)
{
}

void ChatMaster::getDeclaration(const QString& did, const DeclarationCb& successCb, const ErrorCb& errorCb)
{
    qDebug() << "Get declatration" << did;
    mRepoMaster.getRecord<ChatBskyActor::Declaration>(
        did, ATUri::COLLECTION_CHAT_ACTOR_DECLARATION, DECLARATION_KEY, {}, successCb, errorCb);
}

void ChatMaster::updateDeclaration(const QString& did, const ChatBskyActor::Declaration& declaration,
                                   const SuccessCb& successCb, const ErrorCb& errorCb)
{
    mRepoMaster.updateRecord(did, ATUri::COLLECTION_CHAT_ACTOR_DECLARATION, DECLARATION_KEY, declaration, successCb, errorCb);
}

void ChatMaster::createMessage(const QString& text, const std::vector<RichTextMaster::ParsedMatch>& embeddedLinks, const MessageCreatedCb& cb)
{
    Q_ASSERT(cb);
    auto message = std::make_shared<ChatBskyConvo::MessageInput>();
    auto facets = RichTextMaster::parseFacets(text);
    RichTextMaster::insertEmbeddedLinksToFacets(embeddedLinks, facets);

    mRichTextMaster.resolveFacets(text, facets, 0, false,
        [message, cb](const QString& richText, AppBskyRichtext::Facet::List resolvedFacets){
            message->mText = richText;
            message->mFacets = std::move(resolvedFacets);
            cb(message);
        });
}

void ChatMaster::addQuoteToMessage(ChatBskyConvo::MessageInput& message, const QString& quoteUri, const QString& quoteCid)
{
    auto ref = std::make_shared<ComATProtoRepo::StrongRef>();
    ref->mUri = quoteUri;
    ref->mCid = quoteCid;

    Q_ASSERT(!message.mEmbed);
    message.mEmbed = std::make_shared<AppBskyEmbed::Record>();
    message.mEmbed->mRecord = std::move(ref);
}

}
