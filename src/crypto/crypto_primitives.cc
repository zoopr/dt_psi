#include "crypto_primitives.h"
#include <openssl/aead.h>
#include <openssl/curve25519.h>
#include <openssl/hkdf.h>
#include <openssl/rand.h>
#include <openssl/evp.h>


void CryptoPrimitives::x25519_keypair(uint8_t pub[32], uint8_t priv[32]) {
    RAND_bytes(priv, 32);
    X25519_public_from_private(pub, priv);
}

bool CryptoPrimitives::x25519_shared(uint8_t out[32], const uint8_t priv[32], const uint8_t peer_pub[32]) {
    return X25519(out, priv, peer_pub);
}

void CryptoPrimitives::hkdf_sha256(uint8_t *out, size_t out_len, const uint8_t *ikm, size_t ikm_len,
                                   const uint8_t *salt, size_t salt_len, const uint8_t *info, size_t info_len) {
    HKDF(out, out_len, EVP_sha256(), ikm, ikm_len, salt, salt_len, info, info_len);
}

bool CryptoPrimitives::aes_gcm_encrypt(uint8_t *out, size_t *out_len, const uint8_t *plaintext, size_t plaintext_len,
                                       const uint8_t *aad, size_t aad_len, const uint8_t *key, const uint8_t *nonce) {
    // AES-GCM encryption logic using BoringSSL
}

bool CryptoPrimitives::aes_gcm_decrypt(uint8_t *out, size_t *out_len, const uint8_t *ciphertext, size_t ciphertext_len,
                                       const uint8_t *aad, size_t aad_len, const uint8_t *key, const uint8_t *nonce) {
    // AES-GCM decryption logic using BoringSSL
}