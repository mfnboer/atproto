// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QRegularExpression>

namespace ATProto {

// Regular expressions for validating ATProto definitions
class ATRegex {
public:
    static const QRegularExpression HANDLE;
    static const QRegularExpression RKEY;
    static const QRegularExpression DID;

    static bool isValidDid(const QString& did);
};

}
