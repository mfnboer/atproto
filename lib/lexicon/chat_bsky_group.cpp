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

QString joinRuleToString(JoinRule rule)
{
    static const std::unordered_map<JoinRule, QString> mapping ={
        { JoinRule::ANYONE, "anyone" },
        { JoinRule::FOLLOWED_BY_OWNER, "followedByOwner" }
    };

    return enumToString(rule, mapping);
}

JoinLinkView::SharedPtr JoinLinkView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_shared<JoinLinkView>();
    view->mCode = xjson.getRequiredString("code");
    view->mRawEnabledStatus = xjson.getRequiredString("enabledStatus");
    view->mEnabledStatus = stringToLinkEnabledStatus(view->mRawEnabledStatus);
    view->mRequireApproval = xjson.getRequiredBool("requireApproval");
    view->mRawJoinRule = xjson.getRequiredString("joinRule");
    view->mJoinRule = stringToJoinRule(view->mRawJoinRule);
    view->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    return view;
}

JoinLinkViewerState::SharedPtr JoinLinkViewerState::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto state = std::make_shared<JoinLinkViewerState>();
    state->mRequestedAt = xjson.getOptionalDateTime("requestedAt");
    return state;
}

JoinLinkPreviewView::SharedPtr JoinLinkPreviewView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_shared<JoinLinkPreviewView>();
    view->mConvoId = xjson.getRequiredString("convoId");
    view->mCode = xjson.getRequiredString("code");
    view->mName = xjson.getRequiredString("name");
    view->mOwner = xjson.getRequiredObject<ChatBskyActor::ProfileViewBasic>("owner");
    view->mMemberCount = xjson.getRequiredInt("memberCount");
    view->mMemberLimit = xjson.getRequiredInt("memberLimit");
    view->mRequireApproval = xjson.getRequiredBool("requireApproval");
    view->mRawJoinRule = xjson.getRequiredString("joinRule");
    view->mJoinRule = stringToJoinRule(view->mRawJoinRule);
    view->mConvo = xjson.getOptionalObject<ChatBskyConvo::ConvoView>("convo");
    view->mViewer = xjson.getOptionalObject<JoinLinkViewerState>("viewer");
    return view;
}

DisabledJoinLinkPreviewView::SharedPtr DisabledJoinLinkPreviewView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_shared<DisabledJoinLinkPreviewView>();
    view->mCode = xjson.getRequiredString("code");
    return view;
}

InvalidJoinLinkPreviewView::SharedPtr InvalidJoinLinkPreviewView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_shared<InvalidJoinLinkPreviewView>();
    view->mCode = xjson.getRequiredString("code");
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

JoinRequestConvoView::SharedPtr JoinRequestConvoView::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto view = std::make_shared<JoinRequestConvoView>();
    view->mConvoId = xjson.getRequiredString("convoId");
    view->mName = xjson.getRequiredString("name");
    view->mOwner = xjson.getRequiredObject<ChatBskyActor::ProfileViewBasic>("owner");
    view->mMemberCount = xjson.getRequiredInt("memberCount");
    view->mMemberLimit = xjson.getRequiredInt("memberLimit");
    view->mViewer = xjson.getRequiredObject<JoinLinkViewerState>("viewer");
    return view;
}

AddMembersOutput::SharedPtr AddMembersOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<AddMembersOutput>();
    output->mConvo = xjson.getRequiredObject<ChatBskyConvo::ConvoView>("convo");
    output->mAddedMebers = xjson.getOptionalVector<ChatBskyActor::ProfileViewBasic>("addedMembers");
    return output;
}

JoinLinkOutput::SharedPtr JoinLinkOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<JoinLinkOutput>();
    output->mJoinLink = xjson.getRequiredObject<JoinLinkView>("joinLink");
    return output;
}

JoinLinkPreviewsOutput::SharedPtr JoinLinkPreviewsOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<JoinLinkPreviewsOutput>();
    output->mJoinLinkPreviews = xjson.getRequiredVariantList<
        ChatBskyGroup::JoinLinkPreviewView,
        ChatBskyGroup::DisabledJoinLinkPreviewView,
        ChatBskyGroup::InvalidJoinLinkPreviewView,
        UnknownVariant>("joinLinkPreviews");
    return output;
}

JoinRequestsOutput::SharedPtr JoinRequestsOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<JoinRequestsOutput>();
    output->mCursor = xjson.getOptionalString("cursor");
    output->mRequests = xjson.getRequiredVector<JoinRequestView>("requests");
    return output;
}

RequestJoinStatus stringToRequesJoinStatus(const QString& str)
{
    static const std::unordered_map<QString, RequestJoinStatus> mapping = {
        { "joined", RequestJoinStatus::JOINED },
        { "pending", RequestJoinStatus::PENDING }
    };

    return stringToEnum(str, mapping, RequestJoinStatus::UNKNOWN);
}

RequestJoinOutput::SharedPtr RequestJoinOutput::fromJson(const QJsonObject& json)
{
    XJsonObject xjson(json);
    auto output = std::make_shared<RequestJoinOutput>();
    output->mRawStatus = xjson.getRequiredString("status");
    output->mStatus = stringToRequesJoinStatus(output->mRawStatus);
    output->mConvo = xjson.getOptionalObject<ChatBskyConvo::ConvoView>("convo");
    return output;
}

}
