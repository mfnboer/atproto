// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QDateTime>
#include <QException>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>

namespace ATProto
{

class InvalidJsonException : public QException
{
public:
    explicit InvalidJsonException(const QString& msg) : mMsg(msg) {}

    const QString& msg() const { return mMsg; }
    void raise() const override { throw *this; }
    InvalidJsonException *clone() const override { return new InvalidJsonException(*this); }

private:
    QString mMsg;
};

class XJsonObject
{
public:
    XJsonObject(const QJsonObject& obj);

    const QJsonObject& getObject() const { return mObject; }

    QString getRequiredString(const QString& key) const;
    int getRequiredInt(const QString& key) const;
    bool getRequiredBool(const QString& key) const;
    QDateTime getRequiredDateTime(const QString& key) const;
    QJsonObject getRequiredJsonObject(const QString& key) const;
    QJsonArray getRequiredArray(const QString& key) const;
    std::optional<QString> getOptionalString(const QString& key) const;
    QString getOptionalString(const QString& key, const QString& dflt) const;
    std::optional<int> getOptionalInt(const QString& key) const;
    int getOptionalInt(const QString& key, int dflt) const;
    bool getOptionalBool(const QString& key, bool dflt) const;
    std::optional<QDateTime> getOptionalDateTime(const QString& key) const;
    QUrl getOptionalUrl(const QString& key) const;
    std::optional<QJsonObject> getOptionalJsonObject(const QString& key) const;
    std::optional<QJsonArray> getOptionalArray(const QString& key) const;

    template<class ObjType>
    typename ObjType::Ptr getRequiredObject(const QString& key) const;

    template<class ObjType>
    typename ObjType::Ptr getOptionalObject(const QString& key) const;

    template<class ElemType>
    std::vector<typename ElemType::Ptr> getRequiredVector(const QString& key) const;

    template<class ElemType>
    std::vector<typename ElemType::Ptr> getOptionalVector(const QString& key) const;

    std::vector<QString> getRequiredStringVector(const QString& key) const;

private:
    void checkField(const QString& key, QJsonValue::Type type) const;

    const QJsonObject& mObject;
};

template<class ObjType>
typename ObjType::Ptr XJsonObject::getRequiredObject(const QString& key) const
{
    const auto json = getRequiredJsonObject(key);
    return ObjType::fromJson(json);
}

template<class ObjType>
typename ObjType::Ptr XJsonObject::getOptionalObject(const QString& key) const
{
    const auto json = getOptionalJsonObject(key);

    if (!json)
        return nullptr;

    return ObjType::fromJson(*json);
}

template<class ElemType>
std::vector<typename ElemType::Ptr> XJsonObject::getRequiredVector(const QString& key) const
{
    std::vector<typename ElemType::Ptr> result;
    const auto jsonArray = getRequiredArray(key);
    result.reserve(jsonArray.size());

    for (const auto& json : jsonArray)
    {
        if (!json.isObject())
        {
            qWarning() << "PROTO ERROR invalid array element: not an object, key:" << key;
            qInfo() << json;
            throw InvalidJsonException("PROTO ERROR invalid element: " + key);
        }

        typename ElemType::Ptr elem = ElemType::fromJson(json.toObject());
        result.push_back(std::move(elem));
    }

    return result;
}

template<class ElemType>
std::vector<typename ElemType::Ptr> XJsonObject::getOptionalVector(const QString& key) const
{
    std::vector<typename ElemType::Ptr> result;
    const auto jsonArray = getOptionalArray(key);

    if (!jsonArray)
        return result;

    result.reserve(jsonArray->size());

    for (const auto& json : *jsonArray)
    {
        if (!json.isObject())
        {
            qWarning() << "PROTO ERROR invalid array element: not an object, key:" << key;
            qInfo() << json;
            throw InvalidJsonException("PROTO ERROR invalid element: " + key);
        }

        typename ElemType::Ptr elem = ElemType::fromJson(json.toObject());
        result.push_back(std::move(elem));
    }

    return result;
}

}
