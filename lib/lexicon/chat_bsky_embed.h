// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>
#include <QString>

// TODO: unstable
namespace ATProto {

namespace ChatBskyGroup {
struct JoinLinkPreviewView;
}

namespace ChatBskyEmbed {

struct JoinLink
{
    QString mCode;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<JoinLink>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.embed.joinLink";
};

struct JoinLinkView
{
    std::shared_ptr<ChatBskyGroup::JoinLinkPreviewView> mJoinLinkPreview;

    using SharedPtr = std::shared_ptr<JoinLinkView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.embed.joinLink#view";
};

}}
