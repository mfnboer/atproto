// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "com_atproto_server.h"
#include "../xjson.h"

namespace ATProto::ComATProtoServer {

Session::SharedPtr Session::fromJson(const QJsonDocument& json)
{
    const auto jsonObj = json.object();
    const XJsonObject xjson(jsonObj);
    auto session = std::make_shared<Session>();
    session->mHandle = xjson.getRequiredString("handle");
    session->mDid = xjson.getRequiredString("did");
    session->mAccessJwt = xjson.getRequiredString("accessJwt");
    session->mRefreshJwt = xjson.getRequiredString("refreshJwt");
    session->mEmail = xjson.getOptionalString("email");
    session->mEmailConfirmed = xjson.getOptionalBool("emailConfirmed", false);
    session->mEmailAuthFactor = xjson.getOptionalBool("emailAuthFactor", false);
    session->mDidDoc = xjson.getOptionalObject<DidDocument>("didDoc");

    return session;
}

std::optional<QString> Session::getPDS() const
{
    return mDidDoc ? mDidDoc->mATProtoPDS : std::nullopt;
}

GetSessionOutput::SharedPtr GetSessionOutput::fromJson(const QJsonDocument& json)
{
    const auto jsonObj = json.object();
    const XJsonObject xjson(jsonObj);
    auto session = std::make_shared<GetSessionOutput>();
    session->mHandle = xjson.getRequiredString("handle");
    session->mDid = xjson.getRequiredString("did");
    session->mEmail = xjson.getOptionalString("email");
    session->mEmailConfirmed = xjson.getOptionalBool("emailConfirmed", false);
    session->mEmailAuthFactor = xjson.getOptionalBool("emailAuthFactor", false);
    session->mDidDoc = xjson.getOptionalObject<DidDocument>("didDoc");

    return session;
}

InviteCodeUse::SharedPtr InviteCodeUse::fromJson(const QJsonObject& json)
{
    const XJsonObject xjson(json);
    auto inviteCodeUse = std::make_shared<InviteCodeUse>();
    inviteCodeUse->mUsedBy = xjson.getRequiredString("usedBy");
    inviteCodeUse->mUsedAt = xjson.getRequiredDateTime("usedAt");
    return inviteCodeUse;
}

InviteCode::SharedPtr InviteCode::fromJson(const QJsonObject& json)
{
    const XJsonObject xjson(json);
    auto inviteCode = std::make_shared<InviteCode>();
    inviteCode->mCode = xjson.getRequiredString("code");
    inviteCode->mAvailable = xjson.getRequiredInt("available");
    inviteCode->mDisabled = xjson.getRequiredBool("disabled");
    inviteCode->mForAccount = xjson.getRequiredString("forAccount");
    inviteCode->mCreatedBy = xjson.getRequiredString("createdBy");
    inviteCode->mCreatedAt = xjson.getRequiredDateTime("createdAt");
    inviteCode->mUses = xjson.getRequiredVector<InviteCodeUse>("uses");
    return inviteCode;
}

GetAccountInviteCodesOutput::SharedPtr GetAccountInviteCodesOutput::fromJson(const QJsonObject& json)
{
    const XJsonObject xjson(json);
    auto output = std::make_shared<GetAccountInviteCodesOutput>();
    output->mCodes = xjson.getRequiredVector<InviteCode>("codes");
    return output;
}

GetServiceAuthOutput::SharedPtr GetServiceAuthOutput::fromJson(const QJsonObject& json)
{
    const XJsonObject xjson(json);
    auto output = std::make_shared<GetServiceAuthOutput>();
    output->mToken = xjson.getRequiredString("token");
    return output;
}

}
