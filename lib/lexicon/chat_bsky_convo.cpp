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

ReactionViewSender::SharedPtr ReactionViewSender::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto sender = std::make_shared<ReactionViewSender>();
    sender->mDid = xjson.getRequiredString("did");
    return sender;
}

ReactionView::SharedPtr ReactionView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_unique<ReactionView>();
    view->mValue = xjson.getRequiredString("valye");
    view->mSender = xjson.getRequiredObject<ReactionViewSender>("sender");
    view->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return view;
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
    view->mReactions = xjson.getOptionalVector<ReactionView>("reactions");
    view->mSender = xjson.getRequiredObject<MessageViewSender>("sender");
    view->mSentAt = xjson.getRequiredDateTime("sentAt");
    return view;
}

MessageAndReactionView::SharedPtr MessageAndReactionView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_shared<MessageAndReactionView>();
    view->mMessageView = xjson.getRequiredObject<MessageView>("message");
    view->mReactionView = xjson.getRequiredObject<ReactionView>("reaction");
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

ConvoStatus stringToConvoStatus(const QString& str)
{
    static const std::unordered_map<QString, ConvoStatus> mapping = {
        { "request", ConvoStatus::REQUEST },
        { "accepted", ConvoStatus::ACCEPTED }
    };

    return stringToEnum(str, mapping, ConvoStatus::UNKNOWN);
}

QString convoStatusToString(ConvoStatus status)
{
    static const std::unordered_map<ConvoStatus, QString> mapping = {
        { ConvoStatus::REQUEST, "request" },
        { ConvoStatus::ACCEPTED, "accepted" }
    };

    return enumToString(status, mapping);
}

ConvoView::SharedPtr ConvoView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_shared<ConvoView>();
    view->mId = xjson.getRequiredString("id");
    view->mRev = xjson.getRequiredString("rev");
    view->mMembers = xjson.getRequiredVector<ChatBskyActor::ProfileViewBasic>("members");
    view->mLastMessage = xjson.getOptionalVariant<MessageView, DeletedMessageView>("lastMessage");
    view->mLastReaction = xjson.getOptionalVariant<MessageAndReactionView>("lastReaction");
    view->mMuted = xjson.getOptionalBool("muted", false);
    view->mRawStatus = xjson.getOptionalString("status");

    if (view->mRawStatus)
        view->mStatus = stringToConvoStatus(*view->mRawStatus);

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

LogAcceptConvo::SharedPtr LogAcceptConvo::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto logAcceptConvo = std::make_shared<LogAcceptConvo>();
    logAcceptConvo->mConvoId = xjson.getRequiredString("convoId");
    logAcceptConvo->mRev = xjson.getRequiredString("rev");
    return logAcceptConvo;
}

LogLeaveConvo::SharedPtr LogLeaveConvo::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto logLeaveConvo = std::make_shared<LogLeaveConvo>();
    logLeaveConvo->mConvoId = xjson.getRequiredString("convoId");
    logLeaveConvo->mRev = xjson.getRequiredString("rev");
    return logLeaveConvo;
}

LogMuteConvo::SharedPtr LogMuteConvo::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto logMuteConvo = std::make_shared<LogMuteConvo>();
    logMuteConvo->mConvoId = xjson.getRequiredString("convoId");
    logMuteConvo->mRev = xjson.getRequiredString("rev");
    return logMuteConvo;
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

LogReadMessage::SharedPtr LogReadMessage::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto logReadMessage = std::make_shared<LogReadMessage>();
    logReadMessage->mConvoId = xjson.getRequiredString("convoId");
    logReadMessage->mRev = xjson.getRequiredString("rev");
    logReadMessage->mMessage = xjson.getRequiredVariant<MessageView, DeletedMessageView>("message");
    return logReadMessage;
}

AcceptConvoOutput::SharedPtr AcceptConvoOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<AcceptConvoOutput>();
    output->mRev = xjson.getOptionalString("rev");
    return output;
}

ConvoOutput::SharedPtr ConvoOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<ConvoOutput>();
    output->mConvo = xjson.getRequiredObject<ConvoView>("convo");
    return output;
}

ConvoAvailabilityOuput::SharedPtr ConvoAvailabilityOuput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<ConvoAvailabilityOuput>();
    output->mCanChat = xjson.getRequiredBool("canChat");
    output->mConvo = xjson.getOptionalObject<ConvoView>("convo");
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

UpdateAllReadOutput::SharedPtr UpdateAllReadOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<UpdateAllReadOutput>();
    output->mUpdateCount = xjson.getRequiredInt("updateCount");
    return output;
}

MessageOutput::SharedPtr MessageOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<MessageOutput>();
    output->mMessage = xjson.getRequiredObject<MessageView>("message");
    return output;
}

}
