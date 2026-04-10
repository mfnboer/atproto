// Copyright (C) 2026 Michel de Boer
package eu.thereforeiam.atproto;

import android.security.keystore.KeyGenParameterSpec;
import android.security.keystore.KeyProperties;
import android.util.Log;

import java.security.KeyPairGenerator;
import java.security.KeyStore;
import java.security.PrivateKey;
import java.security.PublicKey;
import java.security.Signature;
import java.security.spec.ECGenParameterSpec;

public class KeystoreHelper {
    private static final String PROVIDER = "AndroidKeyStore";
    private static final String LOGTAG = "KeystoreHelper";

    public static boolean generateKey(String alias) {
        try {
            KeyPairGenerator kpg = KeyPairGenerator.getInstance(
                KeyProperties.KEY_ALGORITHM_EC, PROVIDER);

            kpg.initialize(new KeyGenParameterSpec.Builder(
                alias,
                KeyProperties.PURPOSE_SIGN | KeyProperties.PURPOSE_VERIFY)
                .setAlgorithmParameterSpec(new ECGenParameterSpec("secp256r1"))
                .setDigests(KeyProperties.DIGEST_SHA256)
                .build());

            kpg.generateKeyPair();
            return true;
        } catch (Exception e) {
            Log.w(LOGTAG, "Cannot generate key: " + e.getMessage());
            return false;
        }
    }

    public static byte[] sign(String alias, byte[] data) {
        try {
            Log.d(LOGTAG, "Sign: " + alias);
            KeyStore ks = KeyStore.getInstance(PROVIDER);
            ks.load(null);

            PrivateKey privateKey = (PrivateKey) ks.getKey(alias, null);
            Signature sig = Signature.getInstance("SHA256withECDSA");
            sig.initSign(privateKey);
            sig.update(data);
            byte[] signed = sig.sign(); // DER-encoded
            Log.d(LOGTAG, "Signed");
            return signed;
        } catch (Exception e) {
            Log.w(LOGTAG, "Cannot generate key: " + e.getMessage());
            return new byte[0];
        }
    }

    public static byte[] getPublicKey(String alias) {
        try {
            Log.d(LOGTAG, "Get public key: " + alias);
            KeyStore ks = KeyStore.getInstance(PROVIDER);
            ks.load(null);

            byte key[] = ks.getCertificate(alias).getPublicKey().getEncoded(); // DER/X.509
            Log.d(LOGTAG, "Got public key");
            return key;
        } catch (Exception e) {
            Log.w(LOGTAG, "Cannot get public key: " + e.getMessage());
            return new byte[0];
        }
    }

    public static boolean deleteKey(String alias) {
        Log.d(LOGTAG, "Delete key: " + alias);

        try {
            KeyStore ks = KeyStore.getInstance(PROVIDER);
            ks.load(null);
            ks.deleteEntry(alias);
            return true;
        } catch (Exception e) {
            Log.w(LOGTAG, "Cannot delete key: " + e.getMessage());
            return false;
        }
    }
}
