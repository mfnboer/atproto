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

#if defined(Q_OS_ANDROID) && defined(USE_ANDROID_KEYSTORE)
    explicit JsonWebKey(const QString& alias);

    // The alias can be stored to retrieve the key at a later time.
    // The key itself is stored in the Android Keystore.
    const QString& getAlias() const { return mAlias; }
#else
    // Takes ownership of key
    explicit JsonWebKey(EVP_PKEY* key);
#endif

    JsonWebKey() = default;
    JsonWebKey(const JsonWebKey&) = delete;
    JsonWebKey(JsonWebKey&& other);
    ~JsonWebKey();

    JsonWebKey& operator=(const JsonWebKey&) = delete;
    JsonWebKey& operator=(JsonWebKey&& other);

    bool isNull() const;

    QString buildAuthDPoPProof(const QString& httpMethod, const QString& httpUri, const QString& nonce) const;
    QString buildPdsDPoPProof(const QString& httpMethod, const QString& httpUri, const QString& accessToken, const QString& nonce) const;

#if defined(Q_OS_ANDROID) && defined(USE_ANDROID_KEYSTORE)
    // On Android there is no need to save and load the key. The key is stored in
    // the Android keystore. You need to keep the alias for the key to retrieve it.
    static bool deleteKey(const QString& alias);
#else
    // path must be full path with file name.
    bool save(const QString& path, const QString& passPhrase) const;
    static JsonWebKey load(const QString& path, const QString& passPhrase);
    static bool deleteKey(const QString& path);
#endif

private:
    QString buildDPoPProof(const QString& httpMethod, const QString& httpUri, const QString& accessToken, const QString& nonce) const;

    QByteArray sign(const QByteArray& data) const;
    QJsonObject extractPublicJwk() const;

#if defined(Q_OS_ANDROID) && defined(USE_ANDROID_KEYSTORE)
    QString mAlias;
#else
    EVP_PKEY* mKey = nullptr;
#endif
    std::optional<QJsonObject> mPublicJwk;
};

}
