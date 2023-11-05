// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "com_atproto_server.h"
#include "../xjson.h"

namespace ATProto::ComATProtoServer {

Session::Ptr Session::fromJson(const QJsonDocument& json)
{
    const auto jsonObj = json.object();
    const XJsonObject root(jsonObj);
    auto session = std::make_unique<Session>();
    session->mHandle = root.getRequiredString("handle");
    session->mDid = root.getRequiredString("did");
    session->mAccessJwt = root.getRequiredString("accessJwt");
    session->mRefreshJwt = root.getRequiredString("refreshJwt");
    session->mEmail = root.getOptionalString("email");
    session->mEmailConfirmed = root.getOptionalBool("emailConfirmed", false);
    return session;
}

GetSessionOutput::Ptr GetSessionOutput::fromJson(const QJsonDocument& json)
{
    const auto jsonObj = json.object();
    const XJsonObject root(jsonObj);
    auto session = std::make_unique<GetSessionOutput>();
    session->mHandle = root.getRequiredString("handle");
    session->mDid = root.getRequiredString("did");
    session->mEmail = root.getOptionalString("email");
    session->mEmailConfirmed = root.getOptionalBool("emailConfirmed", false);
    return session;
}

InviteCodeUse::Ptr InviteCodeUse::fromJson(const QJsonObject& json)
{
    const XJsonObject xjson(json);
    auto inviteCodeUse = std::make_unique<InviteCodeUse>();
    inviteCodeUse->mUsedBy = xjson.getRequiredString("usedBy");
    inviteCodeUse->mUsedAt = xjson.getRequiredDateTime("usedAt");
    return inviteCodeUse;
}

InviteCode::Ptr InviteCode::fromJson(const QJsonObject& json)
{
    const XJsonObject xjson(json);
    auto inviteCode = std::make_unique<InviteCode>();
    inviteCode->mCode = xjson.getRequiredString("code");
    inviteCode->mAvailable = xjson.getRequiredInt("available");
    inviteCode->mDisabled = xjson.getRequiredBool("disabled");
    inviteCode->mForAccount = xjson.getRequiredString("forAccount");
    inviteCode->mCreatedBy = xjson.getRequiredString("createdBy");
    inviteCode->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    inviteCode->mUses = xjson.getRequiredVector<InviteCodeUse>("uses");
    return inviteCode;
}

GetAccountInviteCodesOutput::Ptr GetAccountInviteCodesOutput::fromJson(const QJsonObject& json)
{
    const XJsonObject xjson(json);
    auto output = std::make_unique<GetAccountInviteCodesOutput>();
    output->mCodes = xjson.getRequiredVector<InviteCode>("codes");
    return output;
}

}
