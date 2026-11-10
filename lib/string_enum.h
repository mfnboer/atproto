// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include "lexicon/lexicon.h"
#include <QString>

namespace ATProto {

template <typename EnumType, EnumType unknownValue>
class StringEnum
{
public:
    static std::unordered_map<QString, EnumType> sStringToEnumMap;

    StringEnum() = default;
    StringEnum(EnumType value, const QString& rawValue = {}) :
        mValue{value},
        mRawValue(rawValue)
    {}

    operator EnumType() const { return mValue; }
    operator int() const { return int(mValue); }

    StringEnum& operator=(EnumType value) { mValue = value; return *this; }

    EnumType value() const { return mValue; }
    const QString& rawValue() const { return mRawValue; }

    static StringEnum fromString(const QString& str)
    {
        return StringEnum{ stringToEnum(str, sStringToEnumMap, unknownValue), str };
    }

    QString toString() const
    {
        return enumToString(mValue, sStringToEnumMap, mRawValue);
    }

private:
    EnumType mValue = EnumType(0);
    QString mRawValue;
};

}