// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>
#include <QString>

// TODO: unstable
namespace ATProto {

namespace ChatBskyGroup {
struct JoinLinkPreviewView;
struct DisabledJoinLinkPreviewView;
struct InvalidJoinLinkPreviewView;
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
    using ViewType = std::variant<
        std::shared_ptr<ChatBskyGroup::JoinLinkPreviewView>,
        std::shared_ptr<ChatBskyGroup::DisabledJoinLinkPreviewView>,
        std::shared_ptr<ChatBskyGroup::InvalidJoinLinkPreviewView>
    >;

    ViewType mJoinLinkPreview;

    using SharedPtr = std::shared_ptr<JoinLinkView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.embed.joinLink#view";
};

}}
