// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "chat_bsky_actor.h"
#include "chat_bsky_group_include.h"
#include "chat_bsky_convo.h"
#include <QJsonDocument>

// TODO: unstable
namespace ATProto::ChatBskyGroup {

struct JoinLinkViewerState {
    std::optional<QDateTime> mRequestedAt;

    using SharedPtr = std::shared_ptr<JoinLinkViewerState>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.group.defs#joinLinkViewerState";
};

struct JoinLinkPreviewView
{
    QString mCode;
    QString mName;
    ChatBskyActor::ProfileViewBasic::SharedPtr mOwner;
    int mMemberCount = 0;
    int mMemberLimit = 0;
    bool mRequireApproval = false;
    QString mRawJoinRule;
    JoinRule mJoinRule;
    QString mRawEnabledStatus;
    LinkEnabledStatus mEnabledStatus;
    ChatBskyConvo::ConvoView::SharedPtr mConvo; // optional
    JoinLinkViewerState::SharedPtr mViewer; // optional

    using SharedPtr = std::shared_ptr<JoinLinkPreviewView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.group.defs#joinLinkPreviewView";
};

struct JoinRequestView
{
    QString mConvoId;
    ChatBskyActor::ProfileViewBasic::SharedPtr mRequestedBy;
    QDateTime mRequestedAt;

    using SharedPtr = std::shared_ptr<JoinRequestView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.group.defs#joinRequestView";
};

struct JoinRequestConvoView
{
    QString mConvoId;
    QString mName;
    ChatBskyActor::ProfileViewBasic::SharedPtr mOwner;
    int mMemberCount = 0;
    int mMemberLimit = 0;
    QDateTime mRequestedAt;

    using SharedPtr = std::shared_ptr<JoinRequestConvoView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.group.defs#joinRequestConvoView";
};

}
