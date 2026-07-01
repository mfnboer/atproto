// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#include "chat_bsky_convo.h"
#include "../xjson.h"

namespace ATProto::ChatBskyConvo {

QJsonObject ConvoRef::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("did", mDid);
    json.insert("convoId", mConvoId);
    return json;
}

ConvoRef::SharedPtr ConvoRef::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto ref = std::make_shared<ConvoRef>();
    ref->mDid = xjson.getRequiredString("did");
    ref->mConvoId = xjson.getRequiredString("convoId");
    return ref;
}

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

QJsonObject ReplyRef::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("messageId", mMessageId);
    return json;
}

ReplyRef::SharedPtr ReplyRef::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto ref = std::make_shared<ReplyRef>();
    ref->mMessageId = xjson.getRequiredString("messageId");
    return ref;
}

QJsonObject MessageInput::toJson() const
{
    QJsonObject json;
    json.insert("$type", MessageInput::TYPE);
    json.insert("text", mText);
    json.insert("facets", XJsonObject::toJsonArray<AppBskyRichtext::Facet>(mFacets));
    XJsonObject::insertOptionalVariant(json, "embed", mEmbed);
    XJsonObject::insertOptionalJsonObject<ReplyRef>(json, "replyTo", mReplyTo);
    return json;
}

MessageInput::SharedPtr MessageInput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto msg = std::make_shared<MessageInput>();
    msg->mText = xjson.getRequiredString("text");
    msg->mFacets = xjson.getOptionalVector<AppBskyRichtext::Facet>("facets");
    msg->mEmbed = xjson.getOptionalVariant<AppBskyEmbed::Record, ChatBskyEmbed::JoinLink>("embed");
    msg->mReplyTo = xjson.getOptionalObject<ReplyRef>("replyTo");
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
    view->mValue = xjson.getRequiredString("value");
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
    view->mEmbed = xjson.getOptionalVariant<AppBskyEmbed::RecordView, ChatBskyEmbed::JoinLinkView>("embed");
    view->mReactions = xjson.getOptionalVector<ReactionView>("reactions");
    view->mReplyTo = xjson.getOptionalVariant<
        MessageView,
        DeletedMessageView,
        MessageBeforeUserJoinedGroupView,
        UnknownVariant>("replyTo");
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

MessageBeforeUserJoinedGroupView::SharedPtr MessageBeforeUserJoinedGroupView::fromJson(const QJsonObject&)
{
    auto view = std::make_shared<MessageBeforeUserJoinedGroupView>();
    return view;
}

SystemMessageReferredUser::SharedPtr SystemMessageReferredUser::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto user = std::make_shared<SystemMessageReferredUser>();
    user->mDid = xjson.getRequiredString("did");
    return user;
}

SystemMessageDataAddMember::SharedPtr SystemMessageDataAddMember::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto data = std::make_shared<SystemMessageDataAddMember>();
    data->mMember = xjson.getRequiredObject<SystemMessageReferredUser>("member");
    data->mRawRole = xjson.getRequiredString("role");
    data->mRole = ChatBskyActor::stringToMemberRole(data->mRawRole);
    data->mAddedBy = xjson.getRequiredObject<SystemMessageReferredUser>("addedBy");
    return data;
}

SystemMessageDataRemoveMember::SharedPtr SystemMessageDataRemoveMember::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto data = std::make_shared<SystemMessageDataRemoveMember>();
    data->mMember = xjson.getRequiredObject<SystemMessageReferredUser>("member");
    data->mRemovedBy = xjson.getRequiredObject<SystemMessageReferredUser>("removedBy");
    return data;
}

SystemMessageDataMemberJoin::SharedPtr SystemMessageDataMemberJoin::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto data = std::make_shared<SystemMessageDataMemberJoin>();
    data->mMember = xjson.getRequiredObject<SystemMessageReferredUser>("member");
    data->mRawRole = xjson.getRequiredString("role");
    data->mRole = ChatBskyActor::stringToMemberRole(data->mRawRole);
    data->mApprovedBy = xjson.getOptionalObject<SystemMessageReferredUser>("approvedBy");
    return data;
}

SystemMessageDataMemberLeave::SharedPtr SystemMessageDataMemberLeave::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto data = std::make_shared<SystemMessageDataMemberLeave>();
    data->mMember = xjson.getRequiredObject<SystemMessageReferredUser>("member");
    return data;
}

SystemMessageDataLockConvo::SharedPtr SystemMessageDataLockConvo::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto data = std::make_shared<SystemMessageDataLockConvo>();
    data->mLockedBy = xjson.getRequiredObject<SystemMessageReferredUser>("lockedBy");
    return data;
}

SystemMessageDataUnlockConvo::SharedPtr SystemMessageDataUnlockConvo::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto data = std::make_shared<SystemMessageDataUnlockConvo>();
    data->mUnlockedBy = xjson.getRequiredObject<SystemMessageReferredUser>("unlockedBy");
    return data;
}

SystemMessageDataLockConvoPermanently::SharedPtr SystemMessageDataLockConvoPermanently::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto data = std::make_shared<SystemMessageDataLockConvoPermanently>();
    data->mLockedBy = xjson.getRequiredObject<SystemMessageReferredUser>("lockedBy");
    return data;
}

SystemMessageDataEditGroup::SharedPtr SystemMessageDataEditGroup::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto data = std::make_shared<SystemMessageDataEditGroup>();
    data->mOldName = xjson.getOptionalString("oldName");
    data->mNewName = xjson.getOptionalString("newName");
    return data;
}

SystemMessageDataCreateJoinLink::SharedPtr SystemMessageDataCreateJoinLink::fromJson(const QJsonObject&)
{
    auto data = std::make_shared<SystemMessageDataCreateJoinLink>();
    return data;
}

SystemMessageDataEditJoinLink::SharedPtr SystemMessageDataEditJoinLink::fromJson(const QJsonObject&)
{
    auto data = std::make_shared<SystemMessageDataEditJoinLink>();
    return data;
}

SystemMessageDataEnableJoinLink::SharedPtr SystemMessageDataEnableJoinLink::fromJson(const QJsonObject&)
{
    auto data = std::make_shared<SystemMessageDataEnableJoinLink>();
    return data;
}

SystemMessageDataDisableJoinLink::SharedPtr SystemMessageDataDisableJoinLink::fromJson(const QJsonObject&)
{
    auto data = std::make_shared<SystemMessageDataDisableJoinLink>();
    return data;
}

SystemMessageView::SharedPtr SystemMessageView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_shared<SystemMessageView>();
    view->mId = xjson.getRequiredString("id");
    view->mRev = xjson.getRequiredString("rev");
    view->mSentAt = xjson.getRequiredDateTime("sentAt");

    view->mData = xjson.getRequiredVariant<
        SystemMessageDataAddMember,
        SystemMessageDataRemoveMember,
        SystemMessageDataMemberJoin,
        SystemMessageDataMemberLeave,
        SystemMessageDataLockConvo,
        SystemMessageDataUnlockConvo,
        SystemMessageDataLockConvoPermanently,
        SystemMessageDataEditGroup,
        SystemMessageDataCreateJoinLink,
        SystemMessageDataEditJoinLink,
        SystemMessageDataEnableJoinLink,
        SystemMessageDataDisableJoinLink,
        UnknownVariant>("data");

    return view;
}

ConvoKind stringToConvoKind(const QString& str)
{
    static const std::unordered_map<QString, ConvoKind> mapping = {
        { "direct", ConvoKind::DIRECT },
        { "group", ConvoKind::GROUP }
    };

    return stringToEnum(str, mapping, ConvoKind::UNKNOWN);
}

QString convoKindToString(ConvoKind kind)
{
    static const std::unordered_map<ConvoKind, QString> mapping = {
        { ConvoKind::DIRECT, "direct" },
        { ConvoKind::GROUP, "group" }
    };

    return enumToString(kind, mapping);
}

ConvoLockStatus stringToConvoLockStatus(const QString& str)
{
    static const std::unordered_map<QString, ConvoLockStatus> mapping = {
        { "unlocked", ConvoLockStatus::UNLOCKED },
        { "locked", ConvoLockStatus::LOCKED },
        { "locked-permanently", ConvoLockStatus::LOCKED_PERMANENTLY }
    };

    return stringToEnum(str, mapping, ConvoLockStatus::UNKNOWN);
}

QString convoLockStatusToString(ConvoLockStatus status)
{
    static const std::unordered_map<ConvoLockStatus, QString> mapping = {
        { ConvoLockStatus::UNLOCKED, "unlocked" },
        { ConvoLockStatus::LOCKED, "locked" },
        { ConvoLockStatus::LOCKED_PERMANENTLY, "locked-permanently" }
    };

    return enumToString(status, mapping);
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

DirectConvo::SharedPtr DirectConvo::fromJson(const QJsonObject&)
{
    auto convo = std::make_shared<DirectConvo>();
    return convo;
}

GroupConvo::SharedPtr GroupConvo::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto convo = std::make_shared<GroupConvo>();
    convo->mName = xjson.getRequiredString("name");
    convo->mMemberCount = xjson.getRequiredInt("memberCount");
    convo->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    convo->mJoinRequestCount = xjson.getOptionalInt("joinRequestCount");
    convo->mUnreadJoinRequestCount = xjson.getOptionalInt("unreadJoinRequestCount");
    convo->mJoinLink = xjson.getOptionalObject<ChatBskyGroup::JoinLinkView>("joinLink");
    convo->mMemberLimit = xjson.getRequiredInt("memberLimit");
    convo->mRawLockStatus = xjson.getRequiredString("lockStatus");
    convo->mLockStatus = stringToConvoLockStatus(convo->mRawLockStatus);
    convo->mLockStatusModerationOverride = xjson.getRequiredBool("lockStatusModerationOverride");
    return convo;
}

ConvoView::SharedPtr ConvoView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_shared<ConvoView>();
    view->mId = xjson.getRequiredString("id");
    view->mRev = xjson.getRequiredString("rev");
    view->mMembers = xjson.getRequiredVector<ChatBskyActor::ProfileViewBasic>("members");
    view->mLastMessage = xjson.getOptionalVariant<MessageView,
                                                  DeletedMessageView,
                                                  SystemMessageView,
                                                  UnknownVariant>("lastMessage");
    view->mLastReaction = xjson.getOptionalVariant<MessageAndReactionView>("lastReaction");
    view->mMuted = xjson.getOptionalBool("muted", false);
    view->mRawStatus = xjson.getOptionalString("status");

    if (view->mRawStatus)
        view->mStatus = stringToConvoStatus(*view->mRawStatus);

    view->mUnreadCount = xjson.getRequiredInt("unreadCount");
    view->mKind = xjson.getOptionalVariant<DirectConvo, GroupConvo>("kind");
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

ConvoUnreadCountsOutput::SharedPtr ConvoUnreadCountsOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<ConvoUnreadCountsOutput>();
    output->mUnreadAcceptedConvos = xjson.getRequiredInt("unreadAcceptedConvos");
    output->mUnreadRequestConvos = xjson.getRequiredInt("unreadRequestConvos");
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

ConvoRequestListOutput::SharedPtr ConvoRequestListOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<ConvoRequestListOutput>();
    output->mCursor = xjson.getOptionalString("cursor");
    output->mRequests = xjson.getRequiredVariantList<ChatBskyConvo::ConvoView,
                                                     ChatBskyGroup::JoinRequestConvoView,
                                                     UnknownVariant>("requests");
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
    output->mMessages = xjson.getRequiredVariantList<MessageView,
                                                     DeletedMessageView,
                                                     SystemMessageView,
                                                     UnknownVariant>("messages");
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

GetConvoMembersOutput::SharedPtr GetConvoMembersOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<GetConvoMembersOutput>();
    output->mCursor = xjson.getOptionalString("cursor");
    output->mMembers = xjson.getRequiredVector<ChatBskyActor::ProfileViewBasic>("members");
    return output;
}

}
