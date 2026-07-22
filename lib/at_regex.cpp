// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "at_regex.h"

namespace ATProto {

const QRegularExpression ATRegex::HANDLE{ R"(([a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?\.)+[a-zA-Z]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?)" };
const QRegularExpression ATRegex::DOMAIN = ATRegex::HANDLE; // a handle is a domain!
const QRegularExpression ATRegex::RKEY{ R"([a-zA-Z0-9\.\-_~:]{1,512})" };
const QRegularExpression ATRegex::DID{ R"(did:[a-z]+:[a-zA-Z0-9\-\.:_]+)"};
const QRegularExpression ATRegex::DID_WEB{ R"(did:web:(?<domain>[a-zA-Z0-9\-\.:_]+))"};

bool ATRegex::isValidDid(const QString& did)
{
    static const QRegularExpression RE_DID(QString(R"(^%1$)").arg(DID.pattern()));
    auto match = RE_DID.matchView(did);
    return match.hasMatch();
}

bool ATRegex::isWebDid(const QString& did)
{
    static const QRegularExpression RE_DID_WEB(QString(R"(^%1$)").arg(DID_WEB.pattern()));
    auto match = RE_DID_WEB.matchView(did);
    return match.hasMatch();
}

bool ATRegex::isHandle(const QString& handle)
{
    static const QRegularExpression RE_HANDLE(QString(R"(^%1$)").arg(HANDLE.pattern()));
    auto match = RE_HANDLE.matchView(handle);
    return match.hasMatch();
}

QString ATRegex::getDomainFromWebDid(const QString& did)
{
    auto match = DID_WEB.matchView(did);
    return match.captured("domain");
}

bool ATRegex::isValidAtprotoProxy(const QString& value)
{
    const auto parts = value.split('#');

    if (parts.size() != 2)
        return false;

    return isValidDid(parts[0]);
}

}
