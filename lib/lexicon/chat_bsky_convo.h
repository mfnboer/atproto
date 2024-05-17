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
    QString mMessageId;

    using Ptr = std::unique_ptr<MessageRef>;
    static Ptr fromJson(const QJsonObject& json);
};

// chat.bsky.convo.defs#message
struct Message
{
    std::optional<QString> mId;
    QString mText; // max 1000 graphemes, 10000 bytes
    AppBskyRichtext::FacetList mFacets;
    ATProto::AppBskyEmbed::Embed::Ptr mEmbed; // optional, only Record

    QJsonObject toJson() const;

    using Ptr = std::unique_ptr<Message>;
    static Ptr fromJson(const QJsonObject& json);
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
    ATProto::AppBskyEmbed::Embed::Ptr mEmbed; // optional, only Record
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

// chat.bsky.convo.defs#controlView
struct ControlView
{
    QString mId;
    QString mRev;
    ChatBskyActor::ProfileViewBasicList mMembers;
    std::optional<std::variant<MessageView::Ptr, DeletedMessageView::Ptr>> mLastMessage;
    bool mMuted = false;
    int mUnreadCount = 0;

    using Ptr = std::unique_ptr<ControlView>;
    static Ptr fromJson(const QJsonObject& json);
};

// chat.bsky.convo.defs#logBeginConvo
struct LogBeginConvo
{
    QString mRev;
    QString mConvoId;

    using Ptr = std::unique_ptr<LogBeginConvo>;
    static Ptr fromJson(const QJsonObject& json);
};

// chat.bsky.convo.defs#logLeaveConvo
struct LogLeaveConvo
{
    QString mRev;
    QString mConvoId;

    using Ptr = std::unique_ptr<LogLeaveConvo>;
    static Ptr fromJson(const QJsonObject& json);
};

// chat.bsky.convo.defs#logCreateMessage
struct LogCreateMessage
{
    QString mRev;
    QString mConvoId;
    std::variant<MessageView::Ptr, DeletedMessageView::Ptr> mMessage; // required

    using Ptr = std::unique_ptr<LogCreateMessage>;
    static Ptr fromJson(const QJsonObject& json);
};

// chat.bsky.convo.defs#logCreateMessage
struct LogDeleteMessage
{
    QString mRev;
    QString mConvoId;
    std::variant<MessageView::Ptr, DeletedMessageView::Ptr> mMessage; // required

    using Ptr = std::unique_ptr<LogDeleteMessage>;
    static Ptr fromJson(const QJsonObject& json);
};

}
