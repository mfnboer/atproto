// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "at_regex.h"

namespace ATProto {

const QRegularExpression ATRegex::HANDLE{ R"(([a-zA-Z0-9]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?\.)+[a-zA-Z]([a-zA-Z0-9\-]{0,61}[a-zA-Z0-9])?)" };
const QRegularExpression ATRegex::RKEY{ R"([a-zA-Z0-9\.\-_~:]{1,512})" };
const QRegularExpression ATRegex::DID{ R"(did:[a-z]+:[a-zA-Z0-9\-\.:_]+)"};

bool ATRegex::isValidDid(const QString& did)
{
    auto match = DID.matchView(did);
    return match.hasMatch();
}

}
