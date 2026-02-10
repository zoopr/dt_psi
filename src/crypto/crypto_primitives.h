#ifndef CRYPTO_PRIMITIVES
#define CRYPTO_PRIMITIVES

#include <cstddef>
#include <cstdint>

class CryptoPrimitives {
public:
    static void x25519_keypair(uint8_t pub[32], uint8_t priv[32]);
    static bool x25519_shared(uint8_t out[32], const uint8_t priv[32], const uint8_t peer_pub[32]);

    static void hkdf_sha256(uint8_t *out, size_t out_len, const uint8_t *ikm, size_t ikm_len,
                            const uint8_t *salt, size_t salt_len, const uint8_t *info, size_t info_len);

    static bool aes_gcm_encrypt(uint8_t *out, size_t *out_len, const uint8_t *plaintext, size_t plaintext_len,
                                 const uint8_t *aad, size_t aad_len, const uint8_t *key, const uint8_t *nonce);

    static bool aes_gcm_decrypt(uint8_t *out, size_t *out_len, const uint8_t *ciphertext, size_t ciphertext_len,
                                 const uint8_t *aad, size_t aad_len, const uint8_t *key, const uint8_t *nonce);
};

#endif