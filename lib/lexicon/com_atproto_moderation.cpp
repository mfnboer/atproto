// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "com_atproto_moderation.h"
#include <QObject>

namespace ATProto::ComATProtoModeration {

QString reasonTypeToString(ReasonType reason)
{
    switch (reason)
    {
    case ReasonType::SPAM:
        return "com.atproto.moderation.defs#reasonSpam";
    case ReasonType::VIOLATION:
        return "com.atproto.moderation.defs#reasonViolation";
    case ReasonType::MISLEADING:
        return "com.atproto.moderation.defs#reasonMisleading";
    case ReasonType::SEXUAL:
        return "com.atproto.moderation.defs#reasonSexual";
    case ReasonType::RUDE:
        return "com.atproto.moderation.defs#reasonRude";
    case ReasonType::OTHER:
        return "com.atproto.moderation.defs#reasonOther";
    }

    Q_ASSERT(false);
    return "UNKNOWN";
}

QString reasonTypeToDescription(ReasonType reason)
{
    switch (reason)
    {
    case ReasonType::SPAM:
        return QObject::tr("Spam: frequent unwanted promotion, replies, mentions");
    case ReasonType::VIOLATION:
        return QObject::tr("Direct violation of server rules, laws, terms of service");
    case ReasonType::MISLEADING:
        return QObject::tr("Misleading identity, affiliation, or content");
    case ReasonType::SEXUAL:
        return QObject::tr("Unwanted or mislabeled sexual content");
    case ReasonType::RUDE:
        return QObject::tr("Rude, harassing, explicit, or otherwise unwelcoming behavior");
    case ReasonType::OTHER:
        return QObject::tr("Other: reports not falling under another report category");
    }

    Q_ASSERT(false);
    return "UNKNOWN";
}

}
