// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "json_web_key.h"
#include <QJsonObject>
#include <QRandomGenerator>
#include <openssl/pem.h>
#include <openssl/ecdsa.h>
#include <openssl/bn.h>
#include <openssl/core_names.h>

namespace ATProto {

// d2i_ECDSA_SIG parses the DER structure and gives you the R and S bignums directly, then BN_bn2binpad writes each one as a fixed-width big-endian byte array into the right half of the output buffer. The same 32-byte width applies here as in extractPublicJwk — it's the field size for P-256, and both functions need to agree on it.
// If you ever switch curves (e.g. to P-384 for ES384), both functions need updating to use 48 bytes instead of 32, and the JWT "alg" claim would change from "ES256" to "ES384" accordingly.
static QByteArray derToRawEcSignature(const QByteArray& der)
{
    const unsigned char* p = reinterpret_cast<const unsigned char*>(der.constData());
    ECDSA_SIG* sig = d2i_ECDSA_SIG(nullptr, &p, der.size());
    if (!sig)
        return {};

    const BIGNUM* r = nullptr;
    const BIGNUM* s = nullptr;
    ECDSA_SIG_get0(sig, &r, &s);

    // Fixed 32 bytes per component for P-256
    QByteArray result(64, '\0');
    BN_bn2binpad(r, reinterpret_cast<unsigned char*>(result.data()),      32);
    BN_bn2binpad(s, reinterpret_cast<unsigned char*>(result.data()) + 32, 32);

    ECDSA_SIG_free(sig);
    return result;
}

QJsonObject extractPublicJwk(EVP_PKEY* pkey) {
    auto bnToBase64Url = [](BIGNUM* bn) -> QString {
        QByteArray buf(32, '\0');
        BN_bn2binpad(bn, reinterpret_cast<unsigned char*>(buf.data()), 32);
        return QString::fromLatin1(
            buf.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals)
            );
    };

    // Extract X and Y directly from the EVP_PKEY
    BIGNUM* x = BN_new();
    BIGNUM* y = BN_new();
    EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_EC_PUB_X, &x);
    EVP_PKEY_get_bn_param(pkey, OSSL_PKEY_PARAM_EC_PUB_Y, &y);

    QJsonObject jwk;
    jwk["kty"] = "EC";
    jwk["crv"] = "P-256";
    jwk["x"]   = bnToBase64Url(x);
    jwk["y"]   = bnToBase64Url(y);

    BN_free(x);
    BN_free(y);

    return jwk;
}

JsonWebKey JsonWebKey::generateDPoPKey()
{
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
    EVP_PKEY_keygen_init(ctx);
    EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, NID_X9_62_prime256v1);

    EVP_PKEY* pkey = nullptr;
    EVP_PKEY_keygen(ctx, &pkey);
    EVP_PKEY_CTX_free(ctx);
    return JsonWebKey(pkey);
}

QByteArray JsonWebKey::sign(const QByteArray& data) const
{
    EVP_MD_CTX* mdCtx = EVP_MD_CTX_new();
    EVP_DigestSignInit(mdCtx, nullptr, EVP_sha256(), nullptr, mKey);
    EVP_DigestSignUpdate(mdCtx, data.constData(), data.size());

    size_t sigLen = 0;
    EVP_DigestSignFinal(mdCtx, nullptr, &sigLen);

    QByteArray sig(sigLen, '\0');
    EVP_DigestSignFinal(mdCtx, reinterpret_cast<unsigned char*>(sig.data()), &sigLen);
    EVP_MD_CTX_free(mdCtx);

    // Convert DER-encoded ECDSA sig to raw R||S for JWT
    return derToRawEcSignature(sig);
}

QString JsonWebKey::buildDPoPProof(const QString& httpMethod, const QString& httpUri, const QString& nonce) const
{
    QJsonObject header;
    header["typ"] = "dpop+jwt";
    header["alg"] = "ES256";
    header["jwk"] = extractPublicJwk(mKey); // public key as JWK

    QJsonObject payload;
    payload["jti"] = generateToken();
    payload["htm"] = httpMethod.toUpper();
    payload["htu"] = httpUri;
    const auto now = QDateTime::currentSecsSinceEpoch();
    payload["iat"] = now;
    payload["exp"] = now + 30;

    if (!nonce.isEmpty()) {
        payload["nonce"] = nonce;
    }

    // Base64url-encode header and payload
    auto b64url = [](const QByteArray& data) {
        return data.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    };

    QByteArray headerB64  = b64url(QJsonDocument(header).toJson(QJsonDocument::Compact));
    QByteArray payloadB64 = b64url(QJsonDocument(payload).toJson(QJsonDocument::Compact));
    QByteArray signingInput = headerB64 + "." + payloadB64;

    // Sign with private key using OpenSSL
    QByteArray signature = sign(signingInput); // see below

    return QString(signingInput + "." + b64url(signature));
}

QString JsonWebKey::generateToken(int length)
{
    Q_ASSERT(length > 0);
    QString token;
    static constexpr char const* tokenSymbols = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    for (int i = 0; i < length; ++i)
    {
        const auto index = QRandomGenerator::global()->bounded(62);
        token += tokenSymbols[index];
    }

    return token;
}

JsonWebKey::JsonWebKey(EVP_PKEY* key) :
    mKey(key)
{
}

JsonWebKey::JsonWebKey(JsonWebKey&& other) :
    mKey(other.mKey)
{
    other.mKey = nullptr;
}

JsonWebKey::~JsonWebKey()
{
    if (mKey)
        EVP_PKEY_free(mKey);
}

JsonWebKey& JsonWebKey::operator=(JsonWebKey&& other)
{
    if (mKey)
        EVP_PKEY_free(mKey);

    mKey = other.mKey;
    other.mKey = nullptr;

    return *this;
}

}
