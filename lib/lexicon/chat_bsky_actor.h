// Copyright (C) 2024 Michel de Boer
// License: GPLv3
#pragma once
#include "app_bsky_actor.h"
#include <QJsonDocument>

namespace ATProto::ChatBskyActor {

// chat.bsky.actor.declaration
struct Declaration
{
    AppBskyActor::AllowIncomingType mAllowIncoming;
    std::optional<AppBskyActor::AllowIncomingType> mAllowGroupInvites;
    QJsonObject mJson;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<Declaration>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.actor.declaration";
};

// chat.bsky.actor.defs#memberRole
enum class MemberRole
{
    OWNER,
    STANDARD,
    UNKNOWN
};

MemberRole stringToMemberRole(const QString& str);

// chat.bsky.actor.defs#directConvoMember
struct DirectConvoMember
{
    using SharedPtr = std::shared_ptr<DirectConvoMember>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.actor.defs#directConvoMember";
};

struct ProfileViewBasic;

// chat.bsky.actor.defs#groupConvoMember
struct GroupConvoMember
{
    // Who added this member. Only present if the member was added (instead of joining via link).
    std::shared_ptr<ProfileViewBasic> mAddedBy;
    QString mRawRole;
    MemberRole mRole = MemberRole::STANDARD;

    using SharedPtr = std::shared_ptr<GroupConvoMember>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.actor.defs#groupConvoMember";
};

// chat.bsky.actor.defs#pastGroupConvoMember
struct PastGroupConvoMember
{
    using SharedPtr = std::shared_ptr<PastGroupConvoMember>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "chat.bsky.actor.defs#pastGroupConvoMember";
};

// chat.bsky.actor.defs#profileViewBasic
struct ProfileViewBasic
{
    QString mDid;
    QString mHandle;
    std::optional<QString> mDisplayName; // max 64 graphemes, 640 bytes
    std::optional<QString> mAvatar; // URL
    AppBskyActor::ProfileAssociated::SharedPtr mAssociated; // optional
    AppBskyActor::ViewerState::SharedPtr mViewer; // optional
    ComATProtoLabel::Label::List mLabels;
    std::optional<QDateTime> mCreatedAt;
    bool mChatDisabled = false;
    AppBskyActor::VerificationState::SharedPtr mVerification; // optional

    using KindType = std::variant<DirectConvoMember::SharedPtr,
                                  GroupConvoMember::SharedPtr,
                                  PastGroupConvoMember::SharedPtr,
                                  UnknownVariant::SharedPtr>;
    std::optional<KindType> mKind;

    using SharedPtr = std::shared_ptr<ProfileViewBasic>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}
