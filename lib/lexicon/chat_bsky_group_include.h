// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "chat_bsky_actor.h"
#include <QDateTime>
#include <QJsonDocument>
#include <QString>

// TODO: unstable
namespace ATProto::ChatBskyGroup {

enum class LinkEnabledStatus
{
    ENABLED,
    DISABLED,
    UNKNOWN
};

LinkEnabledStatus stringToLinkEnabledStatus(const QString& str);

enum class JoinRule
{
    ANYONE,
    FOLLOWED_BY_OWNER,
    UNKNOWN
};

JoinRule stringToJoinRule(const QString& str);
QString joinRuleToString(JoinRule rule);

struct JoinLinkViewerState {
    std::optional<QDateTime> mRequestedAt;

    using SharedPtr = std::shared_ptr<JoinLinkViewerState>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.group.defs#joinLinkViewerState";
};

struct JoinLinkView
{
    QString mCode;
    QString mRawEnabledStatus;
    LinkEnabledStatus mEnabledStatus;
    bool mRequireApproval = false;
    QString mRawJoinRule;
    JoinRule mJoinRule;
    QDateTime mCreatedAt;

    using SharedPtr = std::shared_ptr<JoinLinkView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.group.defs#joinLinkView";
};

struct JoinRequestConvoView
{
    QString mConvoId;
    QString mName;
    ChatBskyActor::ProfileViewBasic::SharedPtr mOwner;
    int mMemberCount = 0;
    int mMemberLimit = 0;
    JoinLinkViewerState::SharedPtr mViewer;

    using SharedPtr = std::shared_ptr<JoinRequestConvoView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.group.defs#joinRequestConvoView";
};

}
