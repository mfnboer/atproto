// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "chat_bsky_actor.h"
#include "chat_bsky_group_include.h"
#include "chat_bsky_convo.h"
#include <QJsonDocument>

// TODO: unstable
namespace ATProto::ChatBskyGroup {

struct JoinLinkPreviewView
{
    QString mConvoId;
    QString mCode;
    QString mName;
    ChatBskyActor::ProfileViewBasic::SharedPtr mOwner;
    int mMemberCount = 0;
    int mMemberLimit = 0;
    bool mRequireApproval = false;
    QString mRawJoinRule;
    JoinRule mJoinRule;
    ChatBskyConvo::ConvoView::SharedPtr mConvo; // optional
    JoinLinkViewerState::SharedPtr mViewer; // optional

    using SharedPtr = std::shared_ptr<JoinLinkPreviewView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.group.defs#joinLinkPreviewView";
};

struct DisabledJoinLinkPreviewView
{
    QString mCode;

    using SharedPtr = std::shared_ptr<DisabledJoinLinkPreviewView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.group.defs#disabledJoinLinkPreviewView";
};

struct InvalidJoinLinkPreviewView
{
    QString mCode;

    using SharedPtr = std::shared_ptr<InvalidJoinLinkPreviewView>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.group.defs#invalidJoinLinkPreviewView";
};

struct JoinRequestView
{
    QString mConvoId;
    ChatBskyActor::ProfileViewBasic::SharedPtr mRequestedBy;
    QDateTime mRequestedAt;

    using SharedPtr = std::shared_ptr<JoinRequestView>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.group.defs#joinRequestView";
};

// chat.bsky.group.addMembers#output
struct AddMembersOutput
{
    ChatBskyConvo::ConvoView::SharedPtr mConvo;
    ChatBskyActor::ProfileViewBasic::List mAddedMebers;

    using SharedPtr = std::shared_ptr<AddMembersOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// chat.bsky.group.createJoinLink#output
struct JoinLinkOutput
{
    JoinLinkView::SharedPtr mJoinLink;

    using SharedPtr = std::shared_ptr<JoinLinkOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// chat.bsky.group.getJoinLinkPreviews#output
struct JoinLinkPreviewsOutput
{
    using ViewType = std::variant<
        std::shared_ptr<ChatBskyGroup::JoinLinkPreviewView>,
        std::shared_ptr<ChatBskyGroup::DisabledJoinLinkPreviewView>,
        std::shared_ptr<ChatBskyGroup::InvalidJoinLinkPreviewView>,
        UnknownVariant::SharedPtr>;
    using ViewList = std::vector<ViewType>;

    ViewList mJoinLinkPreviews;

    using SharedPtr = std::shared_ptr<JoinLinkPreviewsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// chat.bsky.group.listJoinRequests#output
struct JoinRequestsOutput
{
    JoinRequestView::List mRequests;

    using SharedPtr = std::shared_ptr<JoinRequestsOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

enum class RequestJoinStatus
{
    JOINED,
    PENDING,
    UNKNOWN
};

RequestJoinStatus stringToRequesJoinStatus(const QString& str);

// chat.bsky.group.requestJoin#output
struct RequestJoinOutput
{
    QString mRawStatus;
    RequestJoinStatus mStatus;
    ChatBskyConvo::ConvoView::SharedPtr mConvo; // optional

    using SharedPtr = std::shared_ptr<RequestJoinOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}
