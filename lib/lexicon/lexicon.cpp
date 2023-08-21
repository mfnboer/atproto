// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "lexicon.h"
#include "../xjson.h"

namespace ATProto {

ATProtoError::Ptr ATProtoError::fromJson(const QJsonDocument& json)
{
    auto error = std::make_unique<ATProtoError>();
    XJsonObject xjson(json.object());
    error->mError = xjson.getRequiredString("error");
    error->mMessage = xjson.getRequiredString("message");
    return error;
}

RecordType stringToRecordType(const QString& str)
{
    if (str == "app.bsky.feed.post")
        return RecordType::APP_BSKY_FEED_POST;

    qWarning() << "Unknown record type:" << str;
    return RecordType::UNKNOWN;
};

}
