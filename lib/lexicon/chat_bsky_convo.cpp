// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "chat_bsky_convo.h"
#include "../xjson.h"

namespace ATProto::ChatBskyConvo {

QJsonObject MessageRef::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("did", mDid);
    json.insert("convoId", mConvoId);
    json.insert("messageId", mMessageId);
    return json;
}

MessageRef::SharedPtr MessageRef::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto ref = std::make_shared<MessageRef>();
    ref->mDid = xjson.getRequiredString("did");
    ref->mConvoId = xjson.getRequiredString("convoId");
    ref->mMessageId = xjson.getRequiredString("messageId");
    return ref;
}

QJsonObject MessageInput::toJson() const
{
    QJsonObject json;
    json.insert("$type", MessageInput::TYPE);
    json.insert("text", mText);
    json.insert("facets", XJsonObject::toJsonArray<AppBskyRichtext::Facet>(mFacets));
    XJsonObject::insertOptionalJsonObject<AppBskyEmbed::Record>(json, "embed", mEmbed);
    return json;
}

MessageInput::SharedPtr MessageInput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto msg = std::make_shared<MessageInput>();
    msg->mText = xjson.getRequiredString("text");
    msg->mFacets = xjson.getOptionalVector<AppBskyRichtext::Facet>("facets");
    msg->mEmbed = xjson.getOptionalObject<AppBskyEmbed::Record>("embed");
    return msg;
}

MessageViewSender::SharedPtr MessageViewSender::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto sender = std::make_shared<MessageViewSender>();
    sender->mDid = xjson.getRequiredString("did");
    return sender;
}

MessageView::SharedPtr MessageView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_shared<MessageView>();
    view->mId = xjson.getRequiredString("id");
    view->mRev = xjson.getRequiredString("rev");
    view->mText = xjson.getRequiredString("text");
    view->mFacets = xjson.getOptionalVector<AppBskyRichtext::Facet>("facets");
    view->mEmbed = xjson.getOptionalObject<AppBskyEmbed::RecordView>("embed");
    view->mSender = xjson.getRequiredObject<MessageViewSender>("sender");
    view->mSentAt = xjson.getRequiredDateTime("sentAt");
    return view;
}

DeletedMessageView::SharedPtr DeletedMessageView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_shared<DeletedMessageView>();
    view->mId = xjson.getRequiredString("id");
    view->mRev = xjson.getRequiredString("rev");
    view->mSender = xjson.getRequiredObject<MessageViewSender>("sender");
    view->mSentAt = xjson.getRequiredDateTime("sentAt");
    return view;
}

ConvoView::SharedPtr ConvoView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_shared<ConvoView>();
    view->mId = xjson.getRequiredString("id");
    view->mRev = xjson.getRequiredString("rev");
    view->mMembers = xjson.getRequiredVector<ChatBskyActor::ProfileViewBasic>("members");
    view->mLastMessage = xjson.getOptionalVariant<MessageView, DeletedMessageView>("lastMessage");
    view->mMuted = xjson.getOptionalBool("muted", false);
    view->mUnreadCount = xjson.getRequiredInt("unreadCount");
    return view;
}

LogBeginConvo::SharedPtr LogBeginConvo::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto logBeginConvo = std::make_shared<LogBeginConvo>();
    logBeginConvo->mConvoId = xjson.getRequiredString("convoId");
    logBeginConvo->mRev = xjson.getRequiredString("rev");
    return logBeginConvo;
}

LogLeaveConvo::SharedPtr LogLeaveConvo::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto logLeaveConvo = std::make_shared<LogLeaveConvo>();
    logLeaveConvo->mConvoId = xjson.getRequiredString("convoId");
    logLeaveConvo->mRev = xjson.getRequiredString("rev");
    return logLeaveConvo;
}

LogCreateMessage::SharedPtr LogCreateMessage::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto logCreateMessage = std::make_shared<LogCreateMessage>();
    logCreateMessage->mConvoId = xjson.getRequiredString("convoId");
    logCreateMessage->mRev = xjson.getRequiredString("rev");
    logCreateMessage->mMessage = xjson.getRequiredVariant<MessageView, DeletedMessageView>("message");
    return logCreateMessage;
}

LogDeleteMessage::SharedPtr LogDeleteMessage::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto logDeleteMessage = std::make_shared<LogDeleteMessage>();
    logDeleteMessage->mConvoId = xjson.getRequiredString("convoId");
    logDeleteMessage->mRev = xjson.getRequiredString("rev");
    logDeleteMessage->mMessage = xjson.getRequiredVariant<MessageView, DeletedMessageView>("message");
    return logDeleteMessage;
}

ConvoOuput::SharedPtr ConvoOuput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<ConvoOuput>();
    output->mConvo = xjson.getRequiredObject<ConvoView>("convo");
    return output;
}

ConvoListOutput::SharedPtr ConvoListOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<ConvoListOutput>();
    output->mCursor = xjson.getOptionalString("cursor");
    output->mConvos = xjson.getRequiredVector<ConvoView>("convos");
    return output;
}

LogOutput::SharedPtr LogOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<LogOutput>();
    output->mLogs = xjson.getRequiredVariantList<LogBeginConvo, LogLeaveConvo, LogCreateMessage, LogDeleteMessage>("logs");
    return output;
}

GetMessagesOutput::SharedPtr GetMessagesOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<GetMessagesOutput>();
    output->mCursor = xjson.getOptionalString("cursor");
    output->mMessages = xjson.getRequiredVariantList<MessageView, DeletedMessageView>("messages");
    return output;
}

LeaveConvoOutput::SharedPtr LeaveConvoOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<LeaveConvoOutput>();
    output->mConvoId = xjson.getRequiredString("convoId");
    output->mRev = xjson.getRequiredString("rev");
    return output;
}

}
