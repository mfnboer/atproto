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
    AppBskyRichtext::FacetList mFacets;
    ATProto::AppBskyEmbed::Record::SharedPtr mEmbed; // optional

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<MessageInput>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.messageInput";
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
    AppBskyRichtext::FacetList mFacets;
    ATProto::AppBskyEmbed::RecordView::SharedPtr mEmbed; // optional
    MessageViewSender::SharedPtr mSender; // required
    QDateTime mSentAt;

    using SharedPtr = std::shared_ptr<MessageView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#messageView";
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

// chat.bsky.convo.defs#convoView
struct ConvoView
{
    QString mId;
    QString mRev;
    ChatBskyActor::ProfileViewBasicList mMembers;
    std::optional<std::variant<MessageView::SharedPtr, DeletedMessageView::SharedPtr>> mLastMessage;
    bool mMuted = false;
    bool mOpened = false;
    int mUnreadCount = 0;

    using SharedPtr = std::shared_ptr<ConvoView>;
    static SharedPtr fromJson(const QJsonObject& json);
};

using ConvoViewList = std::vector<ConvoView::SharedPtr>;

// chat.bsky.convo.defs#logBeginConvo
struct LogBeginConvo
{
    QString mRev;
    QString mConvoId;

    using SharedPtr = std::shared_ptr<LogBeginConvo>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#logBeginConvo";
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

struct ConvoOuput
{
    ConvoView::SharedPtr mConvo; // required

    using SharedPtr = std::shared_ptr<ConvoOuput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

struct ConvoListOutput
{
    std::optional<QString> mCursor;
    ConvoViewList mConvos;

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

}
