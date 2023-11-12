// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "com_atproto_server.h"
#include "../xjson.h"

namespace ATProto::ComATProtoServer {

Session::Ptr Session::fromJson(const QJsonDocument& json)
{
    const auto jsonObj = json.object();
    const XJsonObject xjson(jsonObj);
    auto session = std::make_unique<Session>();
    session->mHandle = xjson.getRequiredString("handle");
    session->mDid = xjson.getRequiredString("did");
    session->mAccessJwt = xjson.getRequiredString("accessJwt");
    session->mRefreshJwt = xjson.getRequiredString("refreshJwt");
    session->mEmail = xjson.getOptionalString("email");
    session->mEmailConfirmed = xjson.getOptionalBool("emailConfirmed", false);
    auto didDocJson = xjson.getOptionalObject("didDoc");

    if (didDocJson)
    {
        auto didDoc = DidDocument::fromJson(*didDocJson);
        session->mDidDoc = DidDocument::SharedPtr(didDoc.release());
    }

    return session;
}

std::optional<QString> Session::getPDS() const
{
    return mDidDoc ? mDidDoc->mATProtoPDS : std::nullopt;
}

GetSessionOutput::Ptr GetSessionOutput::fromJson(const QJsonDocument& json)
{
    const auto jsonObj = json.object();
    const XJsonObject xjson(jsonObj);
    auto session = std::make_unique<GetSessionOutput>();
    session->mHandle = xjson.getRequiredString("handle");
    session->mDid = xjson.getRequiredString("did");
    session->mEmail = xjson.getOptionalString("email");
    session->mEmailConfirmed = xjson.getOptionalBool("emailConfirmed", false);
    auto didDocJson = xjson.getOptionalObject("didDoc");

    if (didDocJson)
    {
        auto didDoc = DidDocument::fromJson(*didDocJson);
        session->mDidDoc = DidDocument::SharedPtr(didDoc.release());
    }

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
