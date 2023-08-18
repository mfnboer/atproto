// Copyright (C) 2023 Michel de Boer
// License: GPLv3
#pragma once
#include <QException>
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
    XJsonObject(QJsonObject&& obj);

    QString getRequiredString(const QString& key) const;
    int getRequiredInt(const QString& key) const;
    QString getOptionalString(const QString& key, const QString& dflt = {}) const;
    int getOptionalInt(const QString& key, int dflt = 0) const;
    QUrl getOptionalUrl(const QString& key) const;

private:
    void checkField(const QString& key, QJsonValue::Type type) const;

    QJsonObject mObject;
};

}
