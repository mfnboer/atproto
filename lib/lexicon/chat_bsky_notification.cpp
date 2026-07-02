// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "chat_bsky_notification.h"
#include "../xjson.h"

namespace ATProto::ChatBskyNotification {

ChatPreference::IncludeType ChatPreference::stringToIncludeType(const QString& str)
{
    return AppBskyNotification::FilterablePreference::stringToIncludeType(str);
}

QString ChatPreference::includeTypeToString(ChatPreference::IncludeType include, const QString& unknown)
{
    return AppBskyNotification::FilterablePreference::includeTypeToString(include, unknown);
}

QJsonObject ChatPreference::toJson() const
{
    QJsonObject json{mJson};
    json.insert("$type", TYPE);
    json.insert("include", includeTypeToString(mInclude, mRawInclude));
    json.insert("push", mPush);
    return json;
}

ChatPreference::SharedPtr ChatPreference::fromJson(const QJsonObject& json)
{
    auto pref = std::make_shared<ChatPreference>();
    XJsonObject xjson(json);
    pref->mRawInclude = xjson.getRequiredString("include");
    pref->mInclude = stringToIncludeType(pref->mRawInclude);
    pref->mPush = xjson.getRequiredBool("push");
    pref->mJson = json;
    return pref;
}

QJsonObject Preferences::toJson() const
{
    QJsonObject json{mJson};
    json.insert("$type", TYPE);
    json.insert("chat", mChat->toJson());
    json.insert("chat", mChatRequest->toJson());
    return json;
}

Preferences::SharedPtr Preferences::fromJson(const QJsonObject& json)
{
    auto prefs = std::make_shared<Preferences>();
    XJsonObject xjson(json);
    prefs->mChat = xjson.getRequiredObject<ChatPreference>("chat");
    prefs->mChatRequest = xjson.getRequiredObject<ChatPreference>("chatRequest");
    prefs->mJson = json;
    return prefs;
}

GetPreferencesOutput::SharedPtr GetPreferencesOutput::fromJson(const QJsonObject& json)
{
    auto output = std::make_shared<GetPreferencesOutput>();
    XJsonObject xjson(json);
    output->mPreferences = xjson.getRequiredObject<Preferences>("preferences");
    return output;
}

}
