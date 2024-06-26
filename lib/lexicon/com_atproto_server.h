// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "lexicon.h"
#include <QJsonDocument>

namespace ATProto::ComATProtoServer {

// com.atproto.server.createSession#output
struct Session
{
    QString mAccessJwt;
    QString mRefreshJwt;
    QString mHandle;
    QString mDid;
    std::optional<QString> mEmail;
    bool mEmailConfirmed = false;
    bool mEmailAuthFactor = false;
    DidDocument::SharedPtr mDidDoc; // optional

    std::optional<QString> getPDS() const;

    using Ptr = std::unique_ptr<Session>;
    static Ptr fromJson(const QJsonDocument& json);
};

// com.atproto.server.refreshSession#output
struct GetSessionOutput
{
    QString mHandle;
    QString mDid;
    std::optional<QString> mEmail;
    bool mEmailConfirmed = false;
    bool mEmailAuthFactor = false;
    DidDocument::SharedPtr mDidDoc; // optional

    using Ptr = std::unique_ptr<GetSessionOutput>;
    static Ptr fromJson(const QJsonDocument& json);
};

// com.atproto.server.defs#inviteCodeUse
struct InviteCodeUse
{
    QString mUsedBy;
    QDateTime mUsedAt;

    using Ptr = std::unique_ptr<InviteCodeUse>;
    static Ptr fromJson(const QJsonObject& json);
};

// com.atproto.server.defs#inviteCode
struct InviteCode
{
    QString mCode;
    int mAvailable;
    bool mDisabled;
    QString mForAccount;
    QString mCreatedBy;
    QDateTime mCreatedAt;
    std::vector<InviteCodeUse::Ptr> mUses;

    using Ptr = std::unique_ptr<InviteCode>;
    static Ptr fromJson(const QJsonObject& json);
};

// com.atproto.server.getAccountInviteCodes#ouput
struct GetAccountInviteCodesOutput
{
    std::vector<InviteCode::Ptr> mCodes;

    using Ptr = std::unique_ptr<GetAccountInviteCodesOutput>;
    static Ptr fromJson(const QJsonObject& json);
};

}
