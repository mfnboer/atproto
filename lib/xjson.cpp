// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "xjson.h"

namespace ATProto {

XJsonObject::XJsonObject(QJsonObject&& obj) :
    mObject(std::forward<QJsonObject>(obj))
{
}

QString XJsonObject::getRequiredString(const QString& key) const
{
    checkField(key, QJsonValue::String);
    return mObject[key].toString();
}

int XJsonObject::getRequiredInt(const QString& key) const
{
    checkField(key, QJsonValue::Double);
    return mObject[key].toInt();
}

std::optional<QString> XJsonObject::getOptionalString(const QString& key) const
{
    if (mObject.contains(key))
        return mObject[key].toString();

    return {};
}

std::optional<int> XJsonObject::getOptionalInt(const QString& key) const
{
    if (mObject.contains(key))
        return mObject[key].toInt();

    return {};
}

QUrl XJsonObject::getOptionalUrl(const QString& key) const
{
    const QString value = mObject[key].toString();
    const QUrl url(value);

    if (!url.isValid())
        return {};

    return url;
}

void XJsonObject::checkField(const QString& key, QJsonValue::Type type) const
{
    if (!mObject.contains(key))
        throw InvalidJsonException(QString("JSON field missing: %1").arg(key));

    if (mObject.value(key).type() != type)
        throw InvalidJsonException(QString("JSON field %1 does not have type: %2").arg(key).arg(type));
}

}
