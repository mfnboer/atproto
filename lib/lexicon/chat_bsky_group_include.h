// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
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

}
