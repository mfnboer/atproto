// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "chat_bsky_embed.h"
#include "chat_bsky_group.h"
#include "xjson.h"

namespace ATProto::ChatBskyEmbed {

QJsonObject JoinLink::toJson() const
{
    QJsonObject json;
    json.insert("$type", JoinLink::TYPE);
    json.insert("code", mCode);
    return json;
}

JoinLink::SharedPtr JoinLink::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto joinLink = std::make_shared<JoinLink>();
    joinLink->mCode = xjson.getRequiredString("joinLink");
    return joinLink;
}

JoinLinkView::SharedPtr JoinLinkView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_shared<JoinLinkView>();
    view->mJoinLinkPreview = xjson.getRequiredObject<ChatBskyGroup::JoinLinkPreviewView>("joinLinkPreview");
    return view;
}

}
