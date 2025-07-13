// Copyright (C) 2025 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonObject>

namespace ATProto::AppBskyNotification {

// app.bsky.notification.defs#activitySubscription
struct ActivitySubscription
{
    bool mPost;
    bool mReply;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<ActivitySubscription>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "app.bsky.notification.defs#activitySubscription";
};

}
