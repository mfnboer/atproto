// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QString>

namespace ATProto::ComATProtoModeration {

enum class ReasonType
{
    SPAM,
    VIOLATION,
    MISLEADING,
    SEXUAL,
    RUDE,
    OTHER,
    APPEAL
};

QString reasonTypeToString(ReasonType reason);
QString reasonTypeToDescription(ReasonType reason);
QString reasonTypeToTitle(ReasonType reason);

}
