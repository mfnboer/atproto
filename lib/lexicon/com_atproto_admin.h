// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonObject>

namespace ATProto::ComATProtoAdmin {

struct RepoRef
{
    QString mDid;

    QJsonObject toJson() const;

    using SharedPtr = std::shared_ptr<RepoRef>;
    using List = std::vector<SharedPtr>;
    static SharedPtr fromJson(const QJsonObject& json);
    static constexpr char const* TYPE = "com.atproto.admin.defs#repoRef";
};

}