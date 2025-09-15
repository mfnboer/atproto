// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_embed.h"
#include "app_bsky_richtext.h"
#include "chat_bsky_actor.h"
#include <QJsonDocument>

namespace ATProto::ChatBskyConvo {

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

// chat.bsky.convo.defs#messageInput
struct MessageInput
{
    QString mText; // max 1000 graphemes, 10000 bytes
    AppBskyRichtext::Facet::List mFacets;
    ATProto::AppBskyEmbed::Record::SharedPtr mEmbed; // optional

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<MessageInput>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.messageInput";
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

// chat.bsky.convo.defs#messageView
struct MessageView
{
    QString mId;
    QString mRev;
    QString mText; // max 1000 graphemes, 10000 bytes
    AppBskyRichtext::Facet::List mFacets;
    ATProto::AppBskyEmbed::RecordView::SharedPtr mEmbed; // optional
    ReactionView::List mReactions;
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

enum class ConvoStatus
{
    REQUEST,
    ACCEPTED,
    UNKNOWN
};

ConvoStatus stringToConvoStatus(const QString& str);
QString convoStatusToString(ConvoStatus status);

// chat.bsky.convo.defs#convoView
struct ConvoView
{
    using MessageType = std::variant<MessageView::SharedPtr, DeletedMessageView::SharedPtr>;
    using ReactionType = std::variant<MessageAndReactionView::SharedPtr>;

    QString mId;
    QString mRev;
    ChatBskyActor::ProfileViewBasic::List mMembers;
    std::optional<MessageType> mLastMessage;
    std::optional<ReactionType> mLastReaction;
    bool mMuted = false;
    std::optional<QString> mRawStatus;
    std::optional<ConvoStatus> mStatus;
    int mUnreadCount = 0;

    using SharedPtr = std::shared_ptr<ConvoView>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
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

struct ConvoListOutput
{
    std::optional<QString> mCursor;
    ConvoView::List mConvos;

    using SharedPtr = std::shared_ptr<ConvoListOutput>;
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
    using MessageType = std::variant<MessageView::SharedPtr, DeletedMessageView::SharedPtr>;
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

}
