// Copyright (C) 2026 Michel de Boer
// License: GPLv3
#include "json_web_key.h"
#include <QCryptographicHash>
#include <QFile>
#include <QRandomGenerator>
#include <openssl/pem.h>
#include <openssl/ecdsa.h>
#include <openssl/bn.h>
#include <openssl/core_names.h>

#if defined(Q_OS_ANDROID) && defined(USE_ANDROID_KEYSTORE)
#include <QJniObject>
#endif

namespace ATProto {

#if defined(Q_OS_ANDROID) && defined(USE_ANDROID_KEYSTORE)
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
    if (mPublicJwk)
        return *mPublicJwk;

#if defined(Q_OS_ANDROID) && defined(USE_ANDROID_KEYSTORE)
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
    const_cast<JsonWebKey*>(this)->mPublicJwk = extractPublicJwkSsl(pkey);
    EVP_PKEY_free(pkey);
#else
    const_cast<JsonWebKey*>(this)->mPublicJwk = extractPublicJwkSsl(mKey);
#endif

    return *mPublicJwk;
}

JsonWebKey JsonWebKey::generateDPoPKey(const QString& user)
{
#if defined(Q_OS_ANDROID) && defined(USE_ANDROID_KEYSTORE)
    const auto alias = QString("%1-%2").arg(DPOP_KEY_ALIAS, user);
    qDebug() << "Generate DPoP key:" << alias;

    jboolean generated = QJniObject::callStaticMethod<jboolean>(
        "eu/thereforeiam/atproto/KeystoreHelper",
        "generateKey",
        "(Ljava/lang/String;)Z",
        QJniObject::fromString(alias).object()
    );

    if (generated)
        return JsonWebKey(alias);

    return {};
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
#if defined(Q_OS_ANDROID) && defined(USE_ANDROID_KEYSTORE)
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

bool JsonWebKey::isNull() const
{
#if defined(Q_OS_ANDROID) && defined(USE_ANDROID_KEYSTORE)
    return mAlias.isEmpty();
#else
    return mKey == nullptr;
#endif
}

static QString createS256CodeChallend(const QString& token)
{
    const auto hash = QCryptographicHash::hash(token.toUtf8(), QCryptographicHash::Sha256);
    return hash.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
}

QString JsonWebKey::buildDPoPProof(const QString& httpMethod, const QString& httpUri, const QString& accessToken, const QString& nonce) const
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

    if (!accessToken.isEmpty())
        payload["ath"] = createS256CodeChallend(accessToken);

    if (!nonce.isEmpty())
        payload["nonce"] = nonce;

    // Base64url-encode header and payload
    auto b64url = [](const QByteArray& data) {
        return data.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    };

    QByteArray headerB64  = b64url(QJsonDocument(header).toJson(QJsonDocument::Compact));
    QByteArray payloadB64 = b64url(QJsonDocument(payload).toJson(QJsonDocument::Compact));
    QByteArray signingInput = headerB64 + "." + payloadB64;

    // Sign with private key
    QByteArray signature = sign(signingInput);

    const QString proof(signingInput + "." + b64url(signature));
    qDebug() << "End build DPoP proof";
    return proof;
}

QString JsonWebKey::buildAuthDPoPProof(const QString& httpMethod, const QString& httpUri, const QString& nonce) const
{
    qDebug() << "Start build AUTH DPoP proof";
    return buildDPoPProof(httpMethod, httpUri, {}, nonce);
}

QString JsonWebKey::buildPdsDPoPProof(const QString& httpMethod, const QString& httpUri, const QString& accessToken, const QString& nonce) const
{
    qDebug() << "Start build PDS DPoP proof";
    return buildDPoPProof(httpMethod, httpUri, accessToken, nonce);
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

#if defined(Q_OS_ANDROID) && defined(USE_ANDROID_KEYSTORE)
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
#if defined(Q_OS_ANDROID) && defined(USE_ANDROID_KEYSTORE)
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

#if defined(Q_OS_ANDROID) && defined(USE_ANDROID_KEYSTORE)
bool JsonWebKey::deleteKey(const QString& alias)
{
    QJniObject jalias = QJniObject::fromString(alias);
    jboolean deleted = QJniObject::callStaticMethod<jboolean>(
        "eu/thereforeiam/atproto/KeystoreHelper",
        "deleteKey",
        "(Ljava/lang/String;)Z",
        jalias.object()
        );

    return (bool)deleted;
}
#else
bool JsonWebKey::save(const QString& path, const QString& passPhrase) const
{
    qDebug() << "Save:" << path;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
    {
        qWarning() << "Cannot open file:" << path;
        return false;
    }

    const QByteArray passPhraseBytes = passPhrase.toUtf8();
    BIO* bio = BIO_new(BIO_s_mem());
    PEM_write_bio_PrivateKey(bio, mKey, EVP_aes_256_cbc(),
                             nullptr, 0, nullptr, (void*)passPhraseBytes.constData());

    BUF_MEM* mem = nullptr;
    BIO_get_mem_ptr(bio, &mem);
    file.write(mem->data, mem->length);
    BIO_free(bio);
    return true;
}

JsonWebKey JsonWebKey::load(const QString& path, const QString& passPhrase)
{
    qDebug() << "Load:" << path;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << "Cannot open file:" << path;
        return {};
    }

    const QByteArray passPhraseBytes = passPhrase.toUtf8();
    QByteArray data = file.readAll();
    BIO* bio = BIO_new_mem_buf(data.constData(), data.size());
    EVP_PKEY* pkey = PEM_read_bio_PrivateKey(bio, nullptr,
                                             nullptr, (void*)passPhraseBytes.constData());
    BIO_free(bio);
    return JsonWebKey{pkey};
}

bool JsonWebKey::deleteKey(const QString& path)
{
    qDebug() << "Delete:" << path;
    const bool deleted = QFile::remove(path);

    if (!deleted)
        qWarning() << "Failed to delete:" << path;

    return deleted;
}
#endif

}
