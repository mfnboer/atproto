// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_embed.h"
#include "app_bsky_richtext.h"
#include "chat_bsky_actor.h"
#include "chat_bsky_embed.h"
#include "chat_bsky_group_include.h"
#include <QJsonDocument>

namespace ATProto::ChatBskyConvo {

struct ConvoRef
{
    QString mDid;
    QString mConvoId;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ConvoRef>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#convoRef";
};

// chat.bsky.convo.defs#messageRef
struct MessageRef
{
    QString mDid;
    QString mConvoId;
    QString mMessageId;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<MessageRef>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#messageRef";
};

// chat.bsky.convo.defs#replyRef
struct ReplyRef
{
    QString mMessageId;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ReplyRef>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#replyRef";
};

// chat.bsky.convo.defs#messageInput
struct MessageInput
{
    QString mText; // max 1000 graphemes, 10000 bytes
    AppBskyRichtext::Facet::List mFacets;

    // TODO: unstable
    using EmbedType = std::variant<AppBskyEmbed::Record::SharedPtr, ChatBskyEmbed::JoinLink::SharedPtr>;
    std::optional<EmbedType> mEmbed;

    ReplyRef::SharedPtr mReplyTo; // optional

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<MessageInput>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#messageInput";
};

// chat.bsky.convo.defs#reactionViewSender
struct ReactionViewSender
{
    QString mDid;

    using SharedPtr = std::shared_ptr<ReactionViewSender>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// chat.bsky.convo.defs#reactionView
struct ReactionView
{
    QString mValue;
    ReactionViewSender::SharedPtr mSender; // required
    QDateTime mCreatedAt;

    using SharedPtr = std::shared_ptr<ReactionView>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#reactionView";
};

// chat.bsky.convo.defs#messageViewSender
struct MessageViewSender
{
    QString mDid;

    using SharedPtr = std::shared_ptr<MessageViewSender>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct DeletedMessageView
{
    QString mId;
    QString mRev;
    MessageViewSender::SharedPtr mSender; // required
    QDateTime mSentAt;

    using SharedPtr = std::shared_ptr<DeletedMessageView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#deletedMessageView";
};

struct MessageBeforeUserJoinedGroupView {
    using SharedPtr = std::shared_ptr<MessageBeforeUserJoinedGroupView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#,essageBeforeUserJoinedGroupView";
};

// chat.bsky.convo.defs#messageView
struct MessageView
{
    QString mId;
    QString mRev;
    QString mText; // max 1000 graphemes, 10000 bytes
    AppBskyRichtext::Facet::List mFacets;

    using EmbedType = std::variant<AppBskyEmbed::RecordView::SharedPtr, ChatBskyEmbed::JoinLinkView::SharedPtr>;
    std::optional<EmbedType> mEmbed;

    ReactionView::List mReactions;

    using ReplyType = std::variant<std::shared_ptr<MessageView>,
                                   DeletedMessageView::SharedPtr,
                                   MessageBeforeUserJoinedGroupView::SharedPtr,
                                   UnknownVariant::SharedPtr>;

    std::optional<ReplyType> mReplyTo;

    MessageViewSender::SharedPtr mSender; // required
    QDateTime mSentAt;

    using SharedPtr = std::shared_ptr<MessageView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#messageView";
};

// chat.bsky.convo.defs#messageAndReactionView
struct MessageAndReactionView {
    MessageView::SharedPtr mMessageView; // required
    ReactionView::SharedPtr mReactionView; // required

    using SharedPtr = std::shared_ptr<MessageAndReactionView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#messageAndReactionView";
};

// TODO: unstable
struct SystemMessageReferredUser
{
    QString mDid;

    using SharedPtr = std::shared_ptr<SystemMessageReferredUser>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#systemMessageReferredUser";
};

// TODO: unstable
struct SystemMessageDataAddMember
{
    SystemMessageReferredUser::SharedPtr mMember;
    QString mRawRole;
    ChatBskyActor::MemberRole mRole;
    SystemMessageReferredUser::SharedPtr mAddedBy;

    using SharedPtr = std::shared_ptr<SystemMessageDataAddMember>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#systemMessageDataAddMember";
};

// TODO: unstable
struct SystemMessageDataRemoveMember
{
    SystemMessageReferredUser::SharedPtr mMember;
    SystemMessageReferredUser::SharedPtr mRemovedBy;

    using SharedPtr = std::shared_ptr<SystemMessageDataRemoveMember>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#systemMessageDataRemoveMember";
};

// TODO: unstable
struct SystemMessageDataMemberJoin
{
    SystemMessageReferredUser::SharedPtr mMember;
    QString mRawRole;
    ChatBskyActor::MemberRole mRole;
    SystemMessageReferredUser::SharedPtr mApprovedBy; // optional

    using SharedPtr = std::shared_ptr<SystemMessageDataMemberJoin>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#systemMessageDataMemberJoin";
};

// TODO: unstable
struct SystemMessageDataMemberLeave
{
    SystemMessageReferredUser::SharedPtr mMember;

    using SharedPtr = std::shared_ptr<SystemMessageDataMemberLeave>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#systemMessageDataMemberLeave";
};

// TODO: unstable
struct SystemMessageDataLockConvo
{
    SystemMessageReferredUser::SharedPtr mLockedBy;

    using SharedPtr = std::shared_ptr<SystemMessageDataLockConvo>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#systemMessageDataLockConvo";
};

// TODO: unstable
struct SystemMessageDataUnlockConvo
{
    SystemMessageReferredUser::SharedPtr mUnlockedBy;

    using SharedPtr = std::shared_ptr<SystemMessageDataUnlockConvo>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#systemMessageDataUnlockConvo";
};

// TODO: unstable
struct SystemMessageDataLockConvoPermanently
{
    SystemMessageReferredUser::SharedPtr mLockedBy;

    using SharedPtr = std::shared_ptr<SystemMessageDataLockConvoPermanently>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#systemMessageDataLockConvoPermanently";
};

// TODO: unstable
struct SystemMessageDataEditGroup
{
    std::optional<QString> mOldName;
    std::optional<QString> mNewName;

    using SharedPtr = std::shared_ptr<SystemMessageDataEditGroup>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#systemMessageDataEditGroup";
};

// TODO: unstable
struct SystemMessageDataCreateJoinLink
{
    using SharedPtr = std::shared_ptr<SystemMessageDataCreateJoinLink>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#systemMessageDataCreateJoinLink";
};

// TODO: unstable
struct SystemMessageDataEditJoinLink
{
    using SharedPtr = std::shared_ptr<SystemMessageDataEditJoinLink>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#systemMessageDataEditJoinLink";
};

// TODO: unstable
struct SystemMessageDataEnableJoinLink
{
    using SharedPtr = std::shared_ptr<SystemMessageDataEnableJoinLink>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#systemMessageDataEnableJoinLink";
};

// TODO: unstable
struct SystemMessageDataDisableJoinLink
{
    using SharedPtr = std::shared_ptr<SystemMessageDataDisableJoinLink>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#systemMessageDataDisableJoinLink";
};

// TODO: unstable
struct SystemMessageView
{
    QString mId;
    QString mRev;
    QDateTime mSentAt;

    using DataType = std::variant<
        SystemMessageDataAddMember::SharedPtr,
        SystemMessageDataRemoveMember::SharedPtr,
        SystemMessageDataMemberJoin::SharedPtr,
        SystemMessageDataMemberLeave::SharedPtr,
        SystemMessageDataLockConvo::SharedPtr,
        SystemMessageDataUnlockConvo::SharedPtr,
        SystemMessageDataLockConvoPermanently::SharedPtr,
        SystemMessageDataEditGroup::SharedPtr,
        SystemMessageDataCreateJoinLink::SharedPtr,
        SystemMessageDataEditJoinLink::SharedPtr,
        SystemMessageDataEnableJoinLink::SharedPtr,
        SystemMessageDataDisableJoinLink::SharedPtr,
        UnknownVariant::SharedPtr>;

    DataType mData;

    using SharedPtr = std::shared_ptr<SystemMessageView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#systemMessageView";
};

// TODO: unstable
enum class ConvoKind
{
    DIRECT,
    GROUP,
    UNKNOWN
};

ConvoKind stringToConvoKind(const QString& str);
QString convoKindToString(ConvoKind kind);

// TODO: unstable
enum class ConvoLockStatus
{
    UNLOCKED,
    LOCKED,
    LOCKED_PERMANENTLY,
    UNKNOWN
};

ConvoLockStatus stringToConvoLockStatus(const QString& str);
QString convoLockStatusToString(ConvoLockStatus status);

enum class ConvoStatus
{
    REQUEST,
    ACCEPTED,
    UNKNOWN
};

ConvoStatus stringToConvoStatus(const QString& str);
QString convoStatusToString(ConvoStatus status);

// TODO: unstable
struct DirectConvo
{
    using SharedPtr = std::shared_ptr<DirectConvo>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#directConvo";
};

// TODO: unstable
struct GroupConvo
{
    QString mName;
    static constexpr int MAX_GRAPHEMES_NAME = 50;
    static constexpr int MAX_BYTES_NAME = 500;
    int mMemberCount = 0;
    QDateTime mCreatedAt;
    std::optional<int> mJoinRequestCount;
    std::optional<int> mUnreadJoinRequestCount;
    ChatBskyGroup::JoinLinkView::SharedPtr mJoinLink; // optional
    int mMemberLimit = 0;
    QString mRawLockStatus;
    ConvoLockStatus mLockStatus;
    bool mLockStatusModerationOverride = false;

    using SharedPtr = std::shared_ptr<GroupConvo>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#groupConvo";
};

// chat.bsky.convo.defs#convoView
struct ConvoView
{
    using MessageType = std::variant<MessageView::SharedPtr,
                                     DeletedMessageView::SharedPtr,
                                     SystemMessageView::SharedPtr,
                                     UnknownVariant::SharedPtr>;
    using ReactionType = std::variant<MessageAndReactionView::SharedPtr>;
    using KindType = std::variant<DirectConvo::SharedPtr, GroupConvo::SharedPtr>;

    QString mId;
    QString mRev;
    ChatBskyActor::ProfileViewBasic::List mMembers;
    std::optional<MessageType> mLastMessage;
    std::optional<ReactionType> mLastReaction;
    bool mMuted = false;
    std::optional<QString> mRawStatus;
    std::optional<ConvoStatus> mStatus;
    int mUnreadCount = 0;

    // TODO: unstable
    std::optional<KindType> mKind;

    using SharedPtr = std::shared_ptr<ConvoView>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#convoView";
};

// chat.bsky.convo.defs#logBeginConvo
struct LogBeginConvo
{
    QString mRev;
    QString mConvoId;

    using SharedPtr = std::shared_ptr<LogBeginConvo>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#logBeginConvo";
};

// chat.bsky.convo.defs#logAcceptConvo
struct LogAcceptConvo
{
    QString mRev;
    QString mConvoId;

    using SharedPtr = std::shared_ptr<LogAcceptConvo>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#logAcceptConvo";
};

// chat.bsky.convo.defs#logLeaveConvo
struct LogLeaveConvo
{
    QString mRev;
    QString mConvoId;

    using SharedPtr = std::shared_ptr<LogLeaveConvo>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#logLeaveConvo";
};

// chat.bsky.convo.defs#logMuteConvo
struct LogMuteConvo
{
    QString mRev;
    QString mConvoId;

    using SharedPtr = std::shared_ptr<LogMuteConvo>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#logMuteConvo";
};

// chat.bsky.convo.defs#logCreateMessage
struct LogCreateMessage
{
    QString mRev;
    QString mConvoId;

    // null variant for unknown type
    std::variant<MessageView::SharedPtr, DeletedMessageView::SharedPtr> mMessage; // required

    using SharedPtr = std::shared_ptr<LogCreateMessage>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#logCreateMessage";
};

// chat.bsky.convo.defs#logDeleteMessage
struct LogDeleteMessage
{
    QString mRev;
    QString mConvoId;

    // null variant for unknown type
    std::variant<MessageView::SharedPtr, DeletedMessageView::SharedPtr> mMessage; // required

    using SharedPtr = std::shared_ptr<LogDeleteMessage>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#logDeleteMessage";
};

// chat.bsky.convo.defs#logReadMessage
struct LogReadMessage
{
    QString mRev;
    QString mConvoId;

    // null variant for unknown type
    std::variant<MessageView::SharedPtr, DeletedMessageView::SharedPtr> mMessage; // required

    using SharedPtr = std::shared_ptr<LogReadMessage>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#logReadMessage";
};

struct AcceptConvoOutput
{
    std::optional<QString> mRev;

    using SharedPtr = std::shared_ptr<AcceptConvoOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct ConvoOutput
{
    ConvoView::SharedPtr mConvo; // required

    using SharedPtr = std::shared_ptr<ConvoOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct ConvoAvailabilityOuput
{
    bool mCanChat;
    ConvoView::SharedPtr mConvo; // optional

    using SharedPtr = std::shared_ptr<ConvoAvailabilityOuput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct ConvoUnreadCountsOutput
{
    int mUnreadAcceptedConvos = 0;
    int mUnreadRequestConvos = 0;

    using SharedPtr = std::shared_ptr<ConvoUnreadCountsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct ConvoListOutput
{
    std::optional<QString> mCursor;
    ConvoView::List mConvos;

    using SharedPtr = std::shared_ptr<ConvoListOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct ConvoRequestListOutput
{
    using RequestType = std::variant<ChatBskyConvo::ConvoView::SharedPtr,
                                     ChatBskyGroup::JoinRequestConvoView::SharedPtr,
                                     UnknownVariant::SharedPtr>;
    using RequestList = std::vector<RequestType>;

    std::optional<QString> mCursor;
    RequestList mRequests;

    using SharedPtr = std::shared_ptr<ConvoRequestListOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct LogOutput
{
    using LogType = std::variant<LogBeginConvo::SharedPtr, LogLeaveConvo::SharedPtr, LogCreateMessage::SharedPtr, LogDeleteMessage::SharedPtr>;

    std::vector<LogType> mLogs;

    using SharedPtr = std::shared_ptr<LogOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct GetMessagesOutput
{
    using MessageType = ConvoView::MessageType;
    using MessageList = std::vector<MessageType>;

    std::optional<QString> mCursor;
    MessageList mMessages;

    using SharedPtr = std::shared_ptr<GetMessagesOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct LeaveConvoOutput
{
    QString mConvoId;
    QString mRev;

    using SharedPtr = std::shared_ptr<LeaveConvoOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct UpdateAllReadOutput
{
    int mUpdateCount;

    using SharedPtr = std::shared_ptr<UpdateAllReadOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct MessageOutput {
    MessageView::SharedPtr mMessage; // required

    using SharedPtr = std::shared_ptr<MessageOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct GetConvoMembersOutput {
    std::optional<QString> mCursor;
    ChatBskyActor::ProfileViewBasic::List mMembers;

    using SharedPtr = std::shared_ptr<GetConvoMembersOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}
