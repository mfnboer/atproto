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
    mClient(client),
    mRichTextMaster(client)
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

void ChatMaster::createMessage(const QString& text, const MessageCreatedCb& cb)
{
    Q_ASSERT(cb);
    auto message = std::make_shared<ChatBskyConvo::MessageInput>();
    auto facets = RichTextMaster::parseFacets(text);

    mRichTextMaster.resolveFacets(text, facets, 0, false,
        [message, cb](const QString& richText, AppBskyRichtext::FacetList resolvedFacets){
            message->mText = richText;
            message->mFacets = std::move(resolvedFacets);
            cb(message);
        });
}

void ChatMaster::addQuoteToMessage(ChatBskyConvo::MessageInput& message, const QString& quoteUri, const QString& quoteCid)
{
    auto ref = std::make_unique<ComATProtoRepo::StrongRef>();
    ref->mUri = quoteUri;
    ref->mCid = quoteCid;

    Q_ASSERT(!message.mEmbed);
    message.mEmbed = std::make_unique<AppBskyEmbed::Embed>();
    message.mEmbed->mType = AppBskyEmbed::EmbedType::RECORD;
    message.mEmbed->mEmbed = std::make_unique<AppBskyEmbed::Record>();

    auto& record = std::get<AppBskyEmbed::Record::Ptr>(message.mEmbed->mEmbed);
    record->mRecord = std::move(ref);
}

}
