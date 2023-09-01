// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "com_atproto_repo.h"
#include "../xjson.h"

namespace ATProto::ComATProtoRepo {

StrongRef::Ptr StrongRef::fromJson(const QJsonObject& json)
{
    auto strongRef = std::make_unique<StrongRef>();
    XJsonObject xjson(json);
    strongRef->mUri = xjson.getRequiredString("uri");
    strongRef->mCid = xjson.getRequiredString("cid");
    return strongRef;
}

}
