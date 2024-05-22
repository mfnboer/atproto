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

QJsonObject MessageInput::toJson() const
{
    QJsonObject json;
    json.insert("$type", MessageInput::TYPE);
    json.insert("text", mText);
    json.insert("facets", XJsonObject::toJsonArray<AppBskyRichtext::Facet>(mFacets));
    XJsonObject::insertOptionalJsonObject<AppBskyEmbed::Embed>(json, "embed", mEmbed);
    return json;
}

MessageInput::Ptr MessageInput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto msg = std::make_unique<MessageInput>();
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

ConvoView::Ptr ConvoView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_unique<ConvoView>();
    view->mId = xjson.getRequiredString("id");
    view->mRev = xjson.getRequiredString("rev");
    view->mMembers = xjson.getRequiredVector<ChatBskyActor::ProfileViewBasic>("members");
    view->mLastMessage = xjson.getOptionalVariant<MessageView, DeletedMessageView>("lastMessage");
    view->mMuted = xjson.getOptionalBool("muted", false);
    view->mUnreadCount = xjson.getRequiredInt("unreadCount");
    return view;
}

LogBeginConvo::Ptr LogBeginConvo::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto logBeginConvo = std::make_unique<LogBeginConvo>();
    logBeginConvo->mConvoId = xjson.getRequiredString("convoId");
    logBeginConvo->mRev = xjson.getRequiredString("rev");
    return logBeginConvo;
}

LogLeaveConvo::Ptr LogLeaveConvo::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto logLeaveConvo = std::make_unique<LogLeaveConvo>();
    logLeaveConvo->mConvoId = xjson.getRequiredString("convoId");
    logLeaveConvo->mRev = xjson.getRequiredString("rev");
    return logLeaveConvo;
}

LogCreateMessage::Ptr LogCreateMessage::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto logCreateMessage = std::make_unique<LogCreateMessage>();
    logCreateMessage->mConvoId = xjson.getRequiredString("convoId");
    logCreateMessage->mRev = xjson.getRequiredString("rev");
    logCreateMessage->mMessage = xjson.getRequiredVariant<MessageView, DeletedMessageView>("message");
    return logCreateMessage;
}

LogDeleteMessage::Ptr LogDeleteMessage::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto logDeleteMessage = std::make_unique<LogDeleteMessage>();
    logDeleteMessage->mConvoId = xjson.getRequiredString("convoId");
    logDeleteMessage->mRev = xjson.getRequiredString("rev");
    logDeleteMessage->mMessage = xjson.getRequiredVariant<MessageView, DeletedMessageView>("message");
    return logDeleteMessage;
}

ConvoOuput::Ptr ConvoOuput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_unique<ConvoOuput>();
    output->mConvo = xjson.getRequiredObject<ConvoView>("convo");
    return output;
}

ConvoListOutput::Ptr ConvoListOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_unique<ConvoListOutput>();
    output->mCursor = xjson.getOptionalString("cursor");
    output->mConvos = xjson.getRequiredVector<ConvoView>("convos");
    return output;
}

LogOutput::Ptr LogOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_unique<LogOutput>();
    output->mLogs = xjson.getRequiredVariantList<LogBeginConvo, LogLeaveConvo, LogCreateMessage, LogDeleteMessage>("logs");
    return output;
}

GetMessagesOutput::Ptr GetMessagesOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_unique<GetMessagesOutput>();
    output->mCursor = xjson.getOptionalString("cursor");
    output->mMessages = xjson.getRequiredVariantList<MessageView, DeletedMessageView>("messages");
    return output;
}

LeaveConvoOutput::Ptr LeaveConvoOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_unique<LeaveConvoOutput>();
    output->mConvoId = xjson.getRequiredString("convoId");
    output->mRev = xjson.getRequiredString("rev");
    return output;
}

}
