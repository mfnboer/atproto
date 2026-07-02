// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_notification.h"
#include <QJsonDocument>

namespace ATProto::ChatBskyNotification {

struct ChatPreference
{
    using IncludeType = AppBskyNotification::FilterablePreference::IncludeType;

    static IncludeType stringToIncludeType(const QString& str);
    static QString includeTypeToString(IncludeType include, const QString& unknown);

    QString mRawInclude;
    IncludeType mInclude = IncludeType::UNKNOWN;
    bool mPush;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ChatPreference>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.notification.defs#chatPreference";
};

struct Preferences
{
    ChatPreference::SharedPtr mChat;
    ChatPreference::SharedPtr mChatRequest;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Preferences>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.notification.defs#preferences";
};

// chat.bsky.notification.getPreferences#output
struct GetPreferencesOutput
{
    Preferences::SharedPtr mPreferences;

    using SharedPtr = std::shared_ptr<GetPreferencesOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}
