// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <QByteArray>
#include <QJsonObject>
#include <QString>
#include <openssl/ec.h>

namespace ATProto {

class JsonWebKey
{
public:
    static QString generateToken(int length = 30);
    static JsonWebKey generateDPoPKey(const QString& user);

    JsonWebKey() = default;
    JsonWebKey(const JsonWebKey&) = delete;
    JsonWebKey(JsonWebKey&& other);
    ~JsonWebKey();

    JsonWebKey& operator=(const JsonWebKey&) = delete;
    JsonWebKey& operator=(JsonWebKey&& other);

    QString buildDPoPProof(const QString& httpMethod, const QString& httpUri, const QString& nonce) const;

private:
#ifdef Q_OS_ANDROID
    explicit JsonWebKey(const QString& alias);
#else
    explicit JsonWebKey(EVP_PKEY* key);
#endif

    QByteArray sign(const QByteArray& data) const;
    QJsonObject extractPublicJwk() const;

#ifdef Q_OS_ANDROID
    QString mAlias;
#else
    EVP_PKEY* mKey = nullptr;
#endif

};

}
