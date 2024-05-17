// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "chat_bsky_convo.h"
#include "../xjson.h"

namespace ATProto::ChatBskyConvo {

MessageRef::Ptr MessageRef::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto ref = std::make_unique<MessageRef>();
    ref->mDid = xjson.getRequiredString("did");
    ref->mMessageId = xjson.getRequiredString("messageId");
    return ref;
}

QJsonObject Message::toJson() const
{
    QJsonObject json;
    json.insert("$type", "chat.bsky.convo.message");
    XJsonObject::insertOptionalJsonValue(json, "id", mId);
    json.insert("text", mText);
    json.insert("facets", XJsonObject::toJsonArray<AppBskyRichtext::Facet>(mFacets));
    XJsonObject::insertOptionalJsonObject<AppBskyEmbed::Embed>(json, "embed", mEmbed);
    return json;
}

Message::Ptr Message::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto msg = std::make_unique<Message>();
    msg->mId = xjson.getOptionalString("id");
    msg->mText = xjson.getRequiredString("text");
    msg->mFacets = xjson.getOptionalVector<AppBskyRichtext::Facet>("facets");
    msg->mEmbed = xjson.getOptionalObject<AppBskyEmbed::Embed>("embed");
    return msg;
}

MessageViewSender::Ptr MessageViewSender::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto sender = std::make_unique<MessageViewSender>();
    sender->mDid = xjson.getRequiredString("did");
    return sender;
}

MessageView::Ptr MessageView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_unique<MessageView>();
    view->mId = xjson.getRequiredString("id");
    view->mRev = xjson.getRequiredString("rev");
    view->mText = xjson.getRequiredString("text");
    view->mFacets = xjson.getOptionalVector<AppBskyRichtext::Facet>("facets");
    view->mEmbed = xjson.getOptionalObject<AppBskyEmbed::Embed>("embed");
    view->mSender = xjson.getRequiredObject<MessageViewSender>("sender");
    view->mSentAt = xjson.getRequiredDateTime("sentAt");
    return view;
}

DeletedMessageView::Ptr DeletedMessageView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_unique<DeletedMessageView>();
    view->mId = xjson.getRequiredString("id");
    view->mRev = xjson.getRequiredString("rev");
    view->mSender = xjson.getRequiredObject<MessageViewSender>("sender");
    view->mSentAt = xjson.getRequiredDateTime("sentAt");
    return view;
}

ControlView::Ptr ControlView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_unique<ControlView>();
    view->mId = xjson.getRequiredString("id");
    view->mRev = xjson.getRequiredString("rev");
    view->mMembers = xjson.getRequiredVector<ChatBskyActor::ProfileViewBasic>("members");
    view->mLastMessage = xjson.getOptinalVariant<MessageView, DeletedMessageView>("lastMessage");

#if 0
    const auto lastMessageJson = xjson.getOptionalJsonObject("lastMessage");

    if (lastMessageJson)
    {
        const XJsonObject lastMessageXJson(*lastMessageJson);
        const QString lastMessageType = lastMessageXJson.getRequiredString("$type");

        if (lastMessageType == MessageView::TYPE)
            view->mLastMessage = MessageView::fromJson(*lastMessageJson);
        else if (lastMessageType == DeletedMessageView::TYPE)
            view->mLastMessage = DeletedMessageView::fromJson(*lastMessageJson);
        else
            throw InvalidJsonException("Unknown lastMessage type: " + lastMessageType);
    }
#endif

    view->mMuted = xjson.getOptionalBool("muted", false);
    view->mUnreadCount = xjson.getRequiredInt("unreadCount");
    return view;
}

}
