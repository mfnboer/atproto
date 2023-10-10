// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QJsonDocument>

namespace ATProto::ComATProtoServer {

// com.atproto.server.createSession#output
struct Session
{
    QString mAccessJwt;
    QString mRefreshJwt;
    QString mHandle;
    QString mDid;
    std::optional<QString> mEmail;
    bool mEmailConfirmed = false;

    using Ptr = std::unique_ptr<Session>;
    static Ptr fromJson(const QJsonDocument& json);
};

// com.atproto.server.refreshSession#output
struct GetSessionOutput
{
    QString mHandle;
    QString mDid;
    std::optional<QString> mEmail;
    bool mEmailConfirmed = false;

    using Ptr = std::unique_ptr<GetSessionOutput>;
    static Ptr fromJson(const QJsonDocument& json);
};

}
