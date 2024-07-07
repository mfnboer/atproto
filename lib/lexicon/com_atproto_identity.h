// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>

namespace ATProto::ComATProtoIdentity {

// com.atproto.identity.resolveHandle/output
struct ResolveHandleOutput
{
    QString mDid;

    using SharedPtr = std::shared_ptr<ResolveHandleOutput>;
    static SharedPtr fromJson(const QJsonObject& json);
};

}
