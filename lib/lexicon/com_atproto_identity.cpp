// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "com_atproto_identity.h"
#include "../xjson.h"

namespace ATProto::ComATProtoIdentity {

ResolveHandleOutput::SharedPtr ResolveHandleOutput::fromJson(const QJsonObject& json)
{
    auto mention = std::make_shared<ResolveHandleOutput>();
    const XJsonObject root(json);
    mention->mDid = root.getRequiredString("did");
    return mention;
}

}
