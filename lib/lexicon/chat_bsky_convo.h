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

    using Ptr = std::unique_ptr<MessageRef>;
    static Ptr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#messageRef";
};

// chat.bsky.convo.defs#messageInput
struct MessageInput
{
    QString mText; // max 1000 graphemes, 10000 bytes
    AppBskyRichtext::FacetList mFacets;
    ATProto::AppBskyEmbed::Record::Ptr mEmbed; // optional

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<MessageInput>;
    using SharedPtr = std::shared_ptr<MessageInput>;
    static Ptr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.messageInput";
};

// chat.bsky.convo.defs#messageViewSender
struct MessageViewSender
{
    QString mDid;

    using Ptr = std::unique_ptr<MessageViewSender>;
    static Ptr fromJson(const QJsonObject& json);
};

// chat.bsky.convo.defs#messageView
struct MessageView
{
    QString mId;
    QString mRev;
    QString mText; // max 1000 graphemes, 10000 bytes
    AppBskyRichtext::FacetList mFacets;
    ATProto::AppBskyEmbed::RecordView::SharedPtr mEmbed; // optional
    MessageViewSender::Ptr mSender; // required
    QDateTime mSentAt;

    using Ptr = std::unique_ptr<MessageView>;
    static Ptr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#messageView";
};

struct DeletedMessageView
{
    QString mId;
    QString mRev;
    MessageViewSender::Ptr mSender; // required
    QDateTime mSentAt;

    using Ptr = std::unique_ptr<DeletedMessageView>;
    static Ptr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#deletedMessageView";
};

// chat.bsky.convo.defs#convoView
struct ConvoView
{
    QString mId;
    QString mRev;
    ChatBskyActor::ProfileViewBasicList mMembers;
    std::optional<std::variant<MessageView::Ptr, DeletedMessageView::Ptr>> mLastMessage;
    bool mMuted = false;
    int mUnreadCount = 0;

    using Ptr = std::unique_ptr<ConvoView>;
    static Ptr fromJson(const QJsonObject& json);
};

using ConvoViewList = std::vector<ConvoView::Ptr>;

// chat.bsky.convo.defs#logBeginConvo
struct LogBeginConvo
{
    QString mRev;
    QString mConvoId;

    using Ptr = std::unique_ptr<LogBeginConvo>;
    static Ptr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#logBeginConvo";
};

// chat.bsky.convo.defs#logLeaveConvo
struct LogLeaveConvo
{
    QString mRev;
    QString mConvoId;

    using Ptr = std::unique_ptr<LogLeaveConvo>;
    static Ptr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#logLeaveConvo";
};

// chat.bsky.convo.defs#logCreateMessage
struct LogCreateMessage
{
    QString mRev;
    QString mConvoId;
    std::variant<MessageView::Ptr, DeletedMessageView::Ptr> mMessage; // required

    using Ptr = std::unique_ptr<LogCreateMessage>;
    static Ptr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#logCreateMessage";
};

// chat.bsky.convo.defs#logDeleteMessage
struct LogDeleteMessage
{
    QString mRev;
    QString mConvoId;
    std::variant<MessageView::Ptr, DeletedMessageView::Ptr> mMessage; // required

    using Ptr = std::unique_ptr<LogDeleteMessage>;
    static Ptr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.convo.defs#logDeleteMessage";
};

struct ConvoOuput
{
    ConvoView::Ptr mConvo; // required

    using Ptr = std::unique_ptr<ConvoOuput>;
    static Ptr fromJson(const QJsonObject& json);
};

struct ConvoListOutput
{
    std::optional<QString> mCursor;
    ConvoViewList mConvos;

    using Ptr = std::unique_ptr<ConvoListOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

struct LogOutput
{
    using LogType = std::variant<LogBeginConvo::Ptr, LogLeaveConvo::Ptr, LogCreateMessage::Ptr, LogDeleteMessage::Ptr>;

    std::vector<LogType> mLogs;

    using Ptr = std::unique_ptr<LogOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

struct GetMessagesOutput
{
    using MessageType = std::variant<MessageView::Ptr, DeletedMessageView::Ptr>;
    using MessageList = std::vector<MessageType>;

    std::optional<QString> mCursor;
    MessageList mMessages;

    using Ptr = std::unique_ptr<GetMessagesOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

struct LeaveConvoOutput
{
    QString mConvoId;
    QString mRev;

    using Ptr = std::unique_ptr<LeaveConvoOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

}
