// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#include "xjson.h"

namespace ATProto {

XJsonObject::XJsonObject(const QJsonObject& obj) :
    mObject(obj)
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

QDateTime XJsonObject::getRequiredDateTime(const QString& key) const
{
    checkField(key, QJsonValue::String);
    const QString value = mObject[key].toString();
    const QDateTime dateTime = QDateTime::fromString(value, Qt::ISODateWithMs);

    if (!dateTime.isValid())
        throw InvalidJsonException(QString("Invalid datetime: %1").arg(value));

    return dateTime;
}

QJsonObject XJsonObject::getRequiredObject(const QString& key) const
{
    checkField(key, QJsonValue::Object);
    return mObject[key].toObject();
}

QJsonArray XJsonObject::getRequiredArray(const QString& key) const
{
    checkField(key, QJsonValue::Array);
    return mObject[key].toArray();
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

std::optional<QDateTime> XJsonObject::getOptionalDateTime(const QString& key) const
{
    if (mObject.contains(key))
        return getRequiredDateTime(key);

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

std::optional<QJsonObject> XJsonObject::getOptionalObject(const QString& key) const
{
    if (mObject.contains(key))
        return mObject[key].toObject();

    return {};
}

std::optional<QJsonArray> XJsonObject::getOptionalArray(const QString& key) const
{
    if (mObject.contains(key))
        return mObject[key].toArray();

    return {};
}

void XJsonObject::checkField(const QString& key, QJsonValue::Type type) const
{
    if (!mObject.contains(key))
    {
        qWarning() << "Field missing:" << key << mObject;
        throw InvalidJsonException(QString("JSON field missing: %1").arg(key));
    }

    if (mObject.value(key).type() != type)
    {
        qWarning() << "Field:" << key << "has wrong type:" << mObject;
        throw InvalidJsonException(QString("JSON field %1 does not have type: %2").arg(key).arg(type));
    }
}

}
