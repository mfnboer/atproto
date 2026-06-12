// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include "lexicon/lexicon.h"
#include <QDateTime>
#include <QException>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>

namespace ATProto
{

// NOTE: I used to have the complete implementation of this class in the header
// file. It compiled on Linux and Android and worked fine on Linux.
// On Android this somehow lead to two different classes!
// The exception thrown could not be caught with catch (ATProto::InvalidJsonException&)
// The typeid of the exception caught as ATProto::InvalidJsonException but it was
// a different type???
class InvalidJsonException : public QException
{
public:
    explicit InvalidJsonException(const QString& msg);

    const QString& msg() const;
    void raise() const override;
    InvalidJsonException *clone() const override;

private:
    QString mMsg;
};

class XJsonObject
{
public:
    static QJsonArray toJsonArray(const std::vector<QString>& list);

    static void insertOptionalArray(QJsonObject& json, const QString& key, const std::vector<QString>& list);

    template<class Type>
    static QJsonArray toJsonArray(const std::vector<typename Type::SharedPtr>& list);

    template<class Type>
    static void insertOptionalArray(QJsonObject& json, const QString& key, const std::vector<typename Type::SharedPtr>& list);

    template<class... Types>
    static QJsonArray toVariantJsonArray(const std::vector<std::variant<Types...>>& list);

    template<class... Types>
    static void insertOptionalVariantArray(QJsonObject& json, const QString& key, const std::vector<std::variant<Types...>>& list);

    template<class... Types>
    static QJsonObject variantToJsonObject(const std::variant<Types...>& variant);

    template<class... Types>
    static void insertOptionalVariant(QJsonObject& json, const QString& key, const std::optional<std::variant<Types...>>& value);

    template<class Type>
    static void insertOptionalJsonValue(QJsonObject& json, const QString& key, const std::optional<Type>& value);

    template<class Type>
    static void insertOptionalJsonValue(QJsonObject& json, const QString& key, const Type& value, const Type& dflt);

    static void insertOptionalDateTime(QJsonObject& json, const QString& key, const std::optional<QDateTime>& value);

    template<class Type>
    static void insertOptionalJsonObject(QJsonObject& json, const QString& key, const typename Type::SharedPtr& value);

    XJsonObject(const QJsonObject& obj);

    const QJsonObject& getObject() const { return mObject; }

    QString getRequiredString(const QString& key) const;
    int getRequiredInt(const QString& key) const;
    int getRequiredDouble(const QString& key) const;
    bool getRequiredBool(const QString& key) const;
    QDateTime getRequiredDateTime(const QString& key) const;
    QDate getRequiredDate(const QString& key) const;
    QJsonObject getRequiredJsonObject(const QString& key) const;
    QJsonArray getRequiredArray(const QString& key) const;
    std::optional<QString> getOptionalString(const QString& key) const;
    QString getOptionalString(const QString& key, const QString& dflt) const;
    std::optional<int> getOptionalInt(const QString& key) const;
    int getOptionalInt(const QString& key, int dflt) const;
    std::optional<qint64> getOptionalInt64(const QString& key) const;
    qint64 getOptionalInt64(const QString& key, qint64 dflt) const;
    std::optional<bool> getOptionalBool(const QString& key) const;
    bool getOptionalBool(const QString& key, bool dflt) const;
    std::optional<QDateTime> getOptionalDateTime(const QString& key) const;
    QDateTime getOptionalDateTime(const QString& key, QDateTime dflt) const;
    QUrl getOptionalUrl(const QString& key) const;
    std::optional<QJsonObject> getOptionalJsonObject(const QString& key) const;
    std::optional<QJsonArray> getOptionalArray(const QString& key) const;

    template<class ObjType>
    typename ObjType::SharedPtr getRequiredObject(const QString& key) const;

    template<class ObjType>
    typename ObjType::SharedPtr getOptionalObject(const QString& key) const;

    template<class ElemType>
    std::vector<typename ElemType::SharedPtr> getRequiredVector(const QString& key) const;

    template<class ElemType>
    std::vector<typename ElemType::SharedPtr> getOptionalVector(const QString& key) const;

    std::vector<QString> getRequiredStringVector(const QString& key) const;
    std::vector<QString> getOptionalStringVector(const QString& key) const;

    template<typename... Types>
    static std::variant<typename Types::SharedPtr...> toVariant(const QJsonObject& json);

    template<typename... Types>
    std::variant<typename Types::SharedPtr...> getRequiredVariant(const QString& key) const;

    template<typename... Types>
    std::optional<std::variant<typename Types::SharedPtr...>> getOptionalVariant(const QString& key) const;

    template<typename... Types>
    std::vector<std::variant<typename Types::SharedPtr...>> getRequiredVariantList(const QString& key) const;

    template<typename... Types>
    std::vector<std::variant<typename Types::SharedPtr...>> getOptionalVariantList(const QString& key) const;

private:
    void checkField(const QString& key, QJsonValue::Type type) const;

    const QJsonObject& mObject;
};

template<class Type>
QJsonArray XJsonObject::toJsonArray(const std::vector<typename Type::SharedPtr>& list)
{
    QJsonArray jsonArray;

    for (const auto& elem : list)
        jsonArray.append(elem->toJson());

    return jsonArray;
}

template<class Type>
void XJsonObject::insertOptionalArray(QJsonObject& json, const QString& key, const std::vector<typename Type::SharedPtr>& list)
{
    if (list.empty())
        json.remove(key);
    else
        json.insert(key, toJsonArray<Type>(list));
}

template<class Type>
void XJsonObject::insertOptionalJsonValue(QJsonObject& json, const QString& key, const std::optional<Type>& value)
{
    if (value)
        json.insert(key, *value);
    else
        json.remove(key);
}

template<class Type>
void XJsonObject::insertOptionalJsonValue(QJsonObject& json, const QString& key, const Type& value, const Type& dflt)
{
    if (value == dflt)
        json.remove(key);
    else
        json.insert(key, value);
}

template<class Type>
void XJsonObject::insertOptionalJsonObject(QJsonObject& json, const QString& key, const typename Type::SharedPtr& value)
{
    if (value)
        json.insert(key, value->toJson());
    else
        json.remove(key);
}

template<class ObjType>
typename ObjType::SharedPtr XJsonObject::getRequiredObject(const QString& key) const
{
    const auto json = getRequiredJsonObject(key);
    return ObjType::fromJson(json);
}

template<class ObjType>
typename ObjType::SharedPtr XJsonObject::getOptionalObject(const QString& key) const
{
    const auto json = getOptionalJsonObject(key);

    if (!json)
        return nullptr;

    return ObjType::fromJson(*json);
}

template<class ElemType>
std::vector<typename ElemType::SharedPtr> XJsonObject::getRequiredVector(const QString& key) const
{
    std::vector<typename ElemType::SharedPtr> result;
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

        typename ElemType::SharedPtr elem = ElemType::fromJson(json.toObject());
        result.push_back(std::move(elem));
    }

    return result;
}

template<class ElemType>
std::vector<typename ElemType::SharedPtr> XJsonObject::getOptionalVector(const QString& key) const
{
    std::vector<typename ElemType::SharedPtr> result;
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

        typename ElemType::SharedPtr elem = ElemType::fromJson(json.toObject());
        result.push_back(std::move(elem));
    }

    return result;
}

#define VARIANT_CASE(i) \
    case i: \
    { \
        using T = typename std::tuple_element<i < sizeof...(Types) ? i : 0, std::tuple<Types...>>::type; \
        \
        if (type == T::TYPE || UNKNOWN_VARIANT_TYPE == T::TYPE) \
            return T::fromJson(objXJson.getObject()); \
        \
        break; \
    }

template<typename... Types>
std::variant<typename Types::SharedPtr...> XJsonObject::toVariant(const QJsonObject& json)
{
    static_assert(sizeof...(Types) <= 12);
    const QString UNKNOWN_VARIANT_TYPE = "";
    const XJsonObject objXJson(json);
    const QString type = objXJson.getRequiredString("$type");

    for (size_t i = 0; i < sizeof...(Types); ++i)
    {
        switch (i)
        {
            VARIANT_CASE(0)
            VARIANT_CASE(1)
            VARIANT_CASE(2)
            VARIANT_CASE(3)
            VARIANT_CASE(4)
            VARIANT_CASE(5)
            VARIANT_CASE(6)
            VARIANT_CASE(7)
            VARIANT_CASE(8)
            VARIANT_CASE(9)
            VARIANT_CASE(10)
            VARIANT_CASE(11)
        };
    }

    qWarning() << "Unknown type:" << type;
    return {};
}

template<typename... Types>
std::variant<typename Types::SharedPtr...> XJsonObject::getRequiredVariant(const QString& key) const
{
    auto v = toVariant<Types...>(getRequiredJsonObject(key));

    if (isNullVariant(v))
        qWarning() << "Unknown type for key:" << key;

    return v;
}

template<typename... Types>
std::optional<std::variant<typename Types::SharedPtr...>> XJsonObject::getOptionalVariant(const QString& key) const
{
    if (!mObject.contains(key))
        return {};

    auto v = getRequiredVariant<Types...>(key);

    if (isNullVariant(v))
        return {};

    return v;
}

template<typename... Types>
std::vector<std::variant<typename Types::SharedPtr...>> XJsonObject::getRequiredVariantList(const QString& key) const
{
    std::vector<std::variant<typename Types::SharedPtr...>> variantList;
    const auto& arrayJson = getRequiredArray(key);

    for (const auto& arrayElem : arrayJson)
    {
        if (!arrayElem.isObject())
            throw InvalidJsonException("Invalid array element: " + key);

        const auto itemJson = arrayElem.toObject();
        auto item = XJsonObject::toVariant<Types...>(itemJson);

        if (!isNullVariant(item))
            variantList.push_back(std::move(item));
    }

    return variantList;
}

template<typename... Types>
std::vector<std::variant<typename Types::SharedPtr...>> XJsonObject::getOptionalVariantList(const QString& key) const
{
    if (mObject.contains(key))
        return getRequiredVariantList<Types...>(key);

    return {};
}

template<class... Types>
QJsonArray XJsonObject::toVariantJsonArray(const std::vector<std::variant<Types...>>& list)
{
    QJsonArray jsonArray;

    for (const auto& elem : list)
    {
        if (!isNullVariant(elem))
            std::visit([&jsonArray](auto&& x){ jsonArray.append(x->toJson()); }, elem);
    }

    return jsonArray;
}

template<class... Types>
void XJsonObject::insertOptionalVariantArray(QJsonObject& json, const QString& key, const std::vector<std::variant<Types...>>& list)
{
    if (list.empty())
        json.remove(key);
    else
        json.insert(key, toVariantJsonArray(list));
}

template<class... Types>
QJsonObject XJsonObject::variantToJsonObject(const std::variant<Types...>& variant)
{
    Q_ASSERT(!isNullVariant(variant));
    return std::visit([](auto&& x){ return x->toJson(); }, variant);
}

template<class... Types>
void XJsonObject::insertOptionalVariant(QJsonObject& json, const QString& key, const std::optional<std::variant<Types...>>& value)
{
    if (!value)
        json.remove(key);
    else if (isNullVariant(*value))
        qWarning() << "NULL variant:" << key;
    else
        json.insert(key, variantToJsonObject(*value));
}

}
