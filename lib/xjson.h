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
    static QJsonArray toJsonArray(const std::vector<QString>& list);

    static void insertOptionalArray(QJsonObject& json, const QString& key, const std::vector<QString>& list);

    template<class Type>
    static QJsonArray toJsonArray(const std::vector<typename Type::SharedPtr>& list);

    template<class Type>
    static void insertOptionalArray(QJsonObject& json, const QString& key, const std::vector<typename Type::SharedPtr>& list);

    template<class VariantType>
    static QJsonArray toVariantJsonArray(const std::vector<VariantType>& list);

    template<class VariantType>
    static void insertOptionalVariantArray(QJsonObject& json, const QString& key, const std::vector<VariantType>& list);

    template<class Type>
    static void insertOptionalJsonValue(QJsonObject& json, const QString& key, const std::optional<Type>& value);

    template<class Type>
    static void insertOptionalJsonObject(QJsonObject& json, const QString& key, const typename Type::SharedPtr& value);

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
    std::optional<qint64> getOptionalInt64(const QString& key) const;
    qint64 getOptionalInt64(const QString& key, qint64 dflt) const;
    bool getOptionalBool(const QString& key, bool dflt) const;
    std::optional<QDateTime> getOptionalDateTime(const QString& key) const;
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
        if (type == T::TYPE) \
            return T::fromJson(objXJson.getObject()); \
        \
        break; \
    }

template<typename... Types>
std::variant<typename Types::SharedPtr...> XJsonObject::toVariant(const QJsonObject& json)
{
    static_assert(sizeof...(Types) <= 4);
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
    {
        qWarning() << "Unknown type for key:" << key;
    }

    return v;
}

template<typename... Types>
std::optional<std::variant<typename Types::SharedPtr...>> XJsonObject::getOptionalVariant(const QString& key) const
{
    if (mObject.contains(key))
        return getRequiredVariant<Types...>(key);

    return {};
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

template<class VariantType>
QJsonArray XJsonObject::toVariantJsonArray(const std::vector<VariantType>& list)
{
    QJsonArray jsonArray;

    for (const auto& elem : list)
        std::visit([&jsonArray](auto&& x){ jsonArray.append(x->toJson()); }, elem);

    return jsonArray;
}

template<class VariantType>
void XJsonObject::insertOptionalVariantArray(QJsonObject& json, const QString& key, const std::vector<VariantType>& list)
{
    if (list.empty())
        json.remove(key);
    else
        json.insert(key, toVariantJsonArray(list));
}

}
