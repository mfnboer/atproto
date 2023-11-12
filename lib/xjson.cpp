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

bool XJsonObject::getRequiredBool(const QString& key) const
{
    checkField(key, QJsonValue::Bool);
    return mObject[key].toBool();
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

std::vector<QString> XJsonObject::getRequiredStringVector(const QString& key) const
{
    std::vector<QString> result;
    const auto jsonArray = getRequiredArray(key);
    result.reserve(jsonArray.size());

    for (const auto& strJson : jsonArray)
    {
        if (!strJson.isString())
        {
            qWarning() << "Invalid string vector:" << key << "in json:" << mObject;
            throw InvalidJsonException("Invalid string vector: " + key);
        }

        result.push_back(strJson.toString());
    }

    return result;
}

std::optional<QString> XJsonObject::getOptionalString(const QString& key) const
{
    if (mObject.contains(key))
        return mObject[key].toString();

    return {};
}

QString XJsonObject::getOptionalString(const QString& key, const QString& dflt) const
{
    if (mObject.contains(key))
        return mObject[key].toString(dflt);

    return dflt;
}

std::optional<int> XJsonObject::getOptionalInt(const QString& key) const
{
    if (mObject.contains(key))
        return mObject[key].toInt();

    return {};
}

int XJsonObject::getOptionalInt(const QString& key, int dflt) const
{
    if (mObject.contains(key))
        return mObject[key].toInt(dflt);

    return dflt;
}

bool XJsonObject::getOptionalBool(const QString& key, bool dflt) const
{
    if (mObject.contains(key))
        return mObject[key].toBool(dflt);

    return dflt;
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
