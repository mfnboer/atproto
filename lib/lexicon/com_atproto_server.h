// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>

namespace ATProto::ComATProtoServer {

struct Session
{
    QString mAccessJwt;
    QString mRefreshJwt;
    QString mHandle;
    QString mDid;
    std::optional<QString> mEmail;
};

Session fromJson(const QJsonDocument& json);
}
