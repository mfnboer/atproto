// Copyright (C) 2026 Michel de Boer
package eu.thereforeiam.atproto;

import android.security.keystore.KeyGenParameterSpec;
import android.security.keystore.KeyProperties;

import java.security.KeyPairGenerator;
import java.security.KeyStore;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.Signature;
import java.security.spec.ECGenParameterSpec;

public class KeystoreHelper {
    private static final String PROVIDER = "AndroidKeyStore";

    public static void generateKey(String alias) throws Exception {
        KeyPairGenerator kpg = KeyPairGenerator.getInstance(
            KeyProperties.KEY_ALGORITHM_EC, PROVIDER);

        kpg.initialize(new KeyGenParameterSpec.Builder(
            alias,
            KeyProperties.PURPOSE_SIGN | KeyProperties.PURPOSE_VERIFY)
            .setAlgorithmParameterSpec(new ECGenParameterSpec("secp256r1"))
            .setDigests(KeyProperties.DIGEST_SHA256)
            .build());

        kpg.generateKeyPair();
    }

    public static byte[] sign(String alias, byte[] data) throws Exception {
        KeyStore ks = KeyStore.getInstance(PROVIDER);
        ks.load(null);

        PrivateKey privateKey = (PrivateKey) ks.getKey(alias, null);
        Signature sig = Signature.getInstance("SHA256withECDSA");
        sig.initSign(privateKey);
        sig.update(data);
        return sig.sign(); // DER-encoded
    }

    public static byte[] getPublicKey(String alias) throws Exception {
        KeyStore ks = KeyStore.getInstance(PROVIDER);
        ks.load(null);
        return ks.getCertificate(alias).getPublicKey().getEncoded(); // DER/X.509
    }
}
