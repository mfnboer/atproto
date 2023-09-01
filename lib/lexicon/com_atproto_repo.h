// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>

namespace ATProto::ComATProtoRepo {

// com.atproto.repo.strongRef
struct StrongRef
{
    QString mUri;
    QString mCid;

    using Ptr = std::unique_ptr<StrongRef>;
    static Ptr fromJson(const QJsonObject& json);
};

}
