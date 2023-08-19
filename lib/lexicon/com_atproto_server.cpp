// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "com_atproto_server.h"
#include "../xjson.h"

namespace ATProto::ComATProtoServer {

Session fromJson(const QJsonDocument& json)
{
    XJsonObject root(json.object());
    Session session;
    session.mHandle = root.getRequiredString("handle");
    session.mDid = root.getRequiredString("did");
    session.mAccessJwt = root.getRequiredString("accessJwt");
    session.mRefreshJwt = root.getRequiredString("refreshJwt");
    session.mEmail = root.getOptionalString("email");
    return session;
}

}
