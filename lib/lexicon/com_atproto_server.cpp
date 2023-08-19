// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "com_atproto_server.h"
#include "../xjson.h"

namespace ATProto::ComATProtoServer {

Session::Ptr Session::fromJson(const QJsonDocument& json)
{
    XJsonObject root(json.object());
    auto session = std::make_unique<Session>();
    session->mHandle = root.getRequiredString("handle");
    session->mDid = root.getRequiredString("did");
    session->mAccessJwt = root.getRequiredString("accessJwt");
    session->mRefreshJwt = root.getRequiredString("refreshJwt");
    session->mEmail = root.getOptionalString("email");
    return session;
}

}
