// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "chat_bsky_group.h"
#include "../xjson.h"

namespace ATProto::ChatBskyGroup {

LinkEnabledStatus stringToLinkEnabledStatus(const QString& str)
{
    static const std::unordered_map<QString, LinkEnabledStatus> mapping = {
        { "enabled", LinkEnabledStatus::ENABLED },
        { "disabled", LinkEnabledStatus::DISABLED }
    };

    return stringToEnum(str, mapping, LinkEnabledStatus::UNKNOWN);
}

JoinRule stringToJoinRule(const QString& str)
{
    static const std::unordered_map<QString, JoinRule> mapping = {
        { "anyone", JoinRule::ANYONE },
        { "followedByOwner", JoinRule::FOLLOWED_BY_OWNER }
    };

    return stringToEnum(str, mapping, JoinRule::UNKNOWN);
}

JoinLinkView::SharedPtr JoinLinkView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_shared<JoinLinkView>();
    view->mCode = xjson.getRequiredString("code");
    view->mRawEnabledStatus = xjson.getRequiredString("enabledStatus");
    view->mEnabledStatus = stringToLinkEnabledStatus(view->mRawEnabledStatus);
    view->mRawJoinRule = xjson.getRequiredString("joinRule");
    view->mJoinRule = stringToJoinRule(view->mRawJoinRule);
    view->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return view;
}

GroupPublicView::SharedPtr GroupPublicView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_shared<GroupPublicView>();
    view->mName = xjson.getRequiredString("name");
    view->mOwner = xjson.getRequiredObject<ChatBskyActor::ProfileViewBasic>("owner");
    view->mMemberCount = xjson.getRequiredInt("memberCount");
    view->mRequireApproval = xjson.getRequiredBool("requireApproval");
    return view;
}

JoinRequestView::SharedPtr JoinRequestView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_shared<JoinRequestView>();
    view->mConvoId = xjson.getRequiredString("convoId");
    view->mRequestedBy = xjson.getRequiredObject<ChatBskyActor::ProfileViewBasic>("requestedBy");
    view->mRequestedAt = xjson.getRequiredDateTime("requestedAt");
    return view;
}

}
