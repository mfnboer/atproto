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

    using SharedPtr = std::shared_ptr<Session>;
    static SharedPtr fromJson(const QJsonObject& json);
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

    using SharedPtr = std::shared_ptr<GetSessionOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// com.atproto.server.defs#inviteCodeUse
struct InviteCodeUse
{
    QString mUsedBy;
    QDateTime mUsedAt;

    using SharedPtr = std::shared_ptr<InviteCodeUse>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
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
    InviteCodeUse::List mUses;

    using SharedPtr = std::shared_ptr<InviteCode>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// com.atproto.server.getAccountInviteCodes#ouput
struct GetAccountInviteCodesOutput
{
    InviteCode::List mCodes;

    using SharedPtr = std::shared_ptr<GetAccountInviteCodesOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

// com.atproto.server.getServiceAuth#output
struct GetServiceAuthOutput
{
    QString mToken;

    using SharedPtr = std::shared_ptr<GetServiceAuthOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}
