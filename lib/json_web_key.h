// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#pragma once
#include <QByteArray>
#include <QString>
#include <openssl/ec.h>

namespace ATProto {

class JsonWebKey
{
public:
    static QString generateToken(int length = 30);
    static JsonWebKey generateDPoPKey();

    JsonWebKey() = default;
    JsonWebKey(const JsonWebKey&) = delete;
    JsonWebKey(JsonWebKey&& other);
    ~JsonWebKey();

    JsonWebKey& operator=(const JsonWebKey&) = delete;
    JsonWebKey& operator=(JsonWebKey&& other);

    QString buildDPoPProof(const QString& httpMethod, const QString& httpUri, const QString& nonce) const;
    QByteArray sign(const QByteArray& data) const;

private:
    JsonWebKey(EVP_PKEY* key);

    EVP_PKEY* mKey = nullptr;
};

}
