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
    OTHER
};

QString reasonTypeToString(ReasonType reason);
QString reasonToDescription(ReasonType reason);

}
