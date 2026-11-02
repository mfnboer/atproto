// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "com_atproto_admin.h"
#include "../xjson.h"

namespace ATProto::ComATProtoAdmin {

QJsonObject RepoRef::toJson() const
{
    QJsonObject json;
    json.insert("$type", TYPE);
    json.insert("did", mDid);
    return json;
}

RepoRef::SharedPtr RepoRef::fromJson(const QJsonObject& json)
{
    auto repoRef = std::make_shared<RepoRef>();
    const XJsonObject xjson(json);
    repoRef->mDid = xjson.getRequiredString("did");
    return repoRef;
}

}