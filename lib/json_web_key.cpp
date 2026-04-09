// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "json_web_key.h"
#include <QRandomGenerator>
#include <openssl/pem.h>
#include <openssl/ecdsa.h>
#include <openssl/bn.h>
#include <openssl/core_names.h>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#endif

namespace ATProto {

#ifdef Q_OS_ANDROID
constexpr char const* DPOP_KEY_ALIAS = "dpop";
#endif

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

static QJsonObject extractPublicJwkSsl(EVP_PKEY* pkey)
{
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

QJsonObject JsonWebKey::extractPublicJwk() const
{
#ifdef Q_OS_ANDROID
    // Retrieve DER bytes via JNI
    QJniEnvironment env;
    QJniObject jalias = QJniObject::fromString(mAlias);
    QJniObject result = QJniObject::callStaticObjectMethod(
        "eu/thereforeiam/atproto/KeystoreHelper",
        "getPublicKey",
        "(Ljava/lang/String;)[B",
        jalias.object()
    );

    jbyteArray jbytes = result.object<jbyteArray>();
    jsize len = env->GetArrayLength(jbytes);
    QByteArray der(len, '\0');
    env->GetByteArrayRegion(jbytes, 0, len,
                            reinterpret_cast<jbyte*>(der.data()));

    // Parse DER into EVP_PKEY* using OpenSSL
    const unsigned char* p = reinterpret_cast<const unsigned char*>(der.constData());
    EVP_PKEY* pkey = d2i_PUBKEY(nullptr, &p, der.size());
    if (!pkey)
        return {};

    // Reuse your existing function directly
    QJsonObject jwk = extractPublicJwkSsl(pkey);
    EVP_PKEY_free(pkey);
#else
    QJsonObject jwk = extractPublicJwkSsl(mKey);
#endif

    return jwk;
}

JsonWebKey JsonWebKey::generateDPoPKey(const QString& user)
{
#ifdef Q_OS_ANDROID
    const auto alias = QString("%1-%2").arg(DPOP_KEY_ALIAS, user);
    qDebug() << "Generate DPoP key:" << alias;

    QJniObject::callStaticMethod<void>(
        "eu/thereforeiam/atproto/KeystoreHelper",
        "generateKey",
        "(Ljava/lang/String;)V",
        QJniObject::fromString(alias).object()
    );

    return JsonWebKey(alias);
#else
    Q_UNUSED(user);
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
    EVP_PKEY_keygen_init(ctx);
    EVP_PKEY_CTX_set_ec_paramgen_curve_nid(ctx, NID_X9_62_prime256v1);

    EVP_PKEY* pkey = nullptr;
    EVP_PKEY_keygen(ctx, &pkey);
    EVP_PKEY_CTX_free(ctx);
    return JsonWebKey(pkey);
#endif
}

QByteArray JsonWebKey::sign(const QByteArray& data) const
{
#ifdef Q_OS_ANDROID
    QJniObject jalias = QJniObject::fromString(mAlias);
    QJniEnvironment env;

    // Convert data to jbyteArray
    jbyteArray jdata = env->NewByteArray(data.size());
    env->SetByteArrayRegion(jdata, 0, data.size(), reinterpret_cast<const jbyte*>(data.constData()));

    QJniObject result = QJniObject::callStaticObjectMethod(
        "eu/thereforeiam/atproto/KeystoreHelper",
        "sign",
        "(Ljava/lang/String;[B)[B",
        jalias.object(),
        jdata
    );

    env->DeleteLocalRef(jdata);

    if (!result.isValid())
    {
        qWarning() << "Failed to sign data";
        return {};
    }

    jbyteArray jsig = result.object<jbyteArray>();
    jsize len = env->GetArrayLength(jsig);
    QByteArray sig(len, '\0');

    // This comes out as DER, still needs derToRawEcSignature()
    env->GetByteArrayRegion(jsig, 0, len, reinterpret_cast<jbyte*>(sig.data()));
#else
    EVP_MD_CTX* mdCtx = EVP_MD_CTX_new();
    EVP_DigestSignInit(mdCtx, nullptr, EVP_sha256(), nullptr, mKey);
    EVP_DigestSignUpdate(mdCtx, data.constData(), data.size());

    size_t sigLen = 0;
    EVP_DigestSignFinal(mdCtx, nullptr, &sigLen);

    QByteArray sig(sigLen, '\0');
    EVP_DigestSignFinal(mdCtx, reinterpret_cast<unsigned char*>(sig.data()), &sigLen);
    EVP_MD_CTX_free(mdCtx);
#endif

    // Convert DER-encoded ECDSA sig to raw R||S for JWT
    return derToRawEcSignature(sig);
}

QString JsonWebKey::buildDPoPProof(const QString& httpMethod, const QString& httpUri, const QString& nonce) const
{
    QJsonObject header;
    header["typ"] = "dpop+jwt";
    header["alg"] = "ES256";
    header["jwk"] = extractPublicJwk(); // public key as JWK

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

    // Sign with private key
    QByteArray signature = sign(signingInput);

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

#ifdef Q_OS_ANDROID
JsonWebKey::JsonWebKey(const QString& alias) :
    mAlias(alias)
{
}

JsonWebKey::JsonWebKey(JsonWebKey&& other) :
    mAlias(other.mAlias)
{
    other.mAlias.clear();
}

JsonWebKey::~JsonWebKey()
{
}
#else
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
#endif

JsonWebKey& JsonWebKey::operator=(JsonWebKey&& other)
{
#ifdef Q_OS_ANDROID
    mAlias = other.mAlias;
    other.mAlias.clear();
#else
    if (mKey)
        EVP_PKEY_free(mKey);

    mKey = other.mKey;
    other.mKey = nullptr;
#endif

    return *this;
}

}
