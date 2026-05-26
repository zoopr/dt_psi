#ifndef CRYPTO_PRIMITIVES
#define CRYPTO_PRIMITIVES

#include <cstddef>
#include <cstdint>
#include <vector>
#include <set>
#include <optional>
#include <chrono>

#include "crypto_types.h"

class CryptoPrimitives {
public:
// X25519 Keypair primitives
    static void x25519_keypair(uint8_t pub[32], uint8_t priv[32]);
    static bool x25519_shared(uint8_t out[32], const uint8_t priv[32], const uint8_t peer_pub[32]);
// KDF primitive
    static void hkdf_sha256(uint8_t *out, size_t out_len, const uint8_t *ikm, size_t ikm_len,
                            const uint8_t *salt, size_t salt_len, const uint8_t *info, size_t info_len);
// AES-GCM AEAD primitive
    static bool aes_gcm_encrypt(std::vector<uint8_t> *out, size_t out_len, const uint8_t *plaintext, size_t plaintext_len,
                                 const uint8_t *aad, size_t aad_len, const uint8_t *key, uint8_t *nonce);

    static bool aes_gcm_decrypt(std::vector<uint8_t> *out, size_t out_len, const uint8_t *ciphertext, size_t ciphertext_len,
                                 const uint8_t *aad, size_t aad_len, const uint8_t *key, const uint8_t *nonce);
// MAC primitive
    static bool aes_hmac_tag(uint8_t *out, size_t* out_len, const uint8_t *key, const size_t key_len, const uint8_t *data, const size_t data_len);
// SSS primitives
    static bool sss_share_gen(uint8_t *out, size_t out_len, uint8_t *data, const uint8_t num_shares, const uint8_t threshold);

    static bool sss_share_reconstruct(uint8_t *out, size_t out_len, uint8_t *in, size_t in_len, const uint8_t num_shares);
// Crypto-secure random sources and misc.
    static uint64_t secure_random_uint64(const uint64_t max);

    static void gen_dummy(uint8_t *out, size_t out_len);

    static bool secure_compare(uint8_t* b1, uint8_t* b2, size_t len);

    static void print_stats(std::vector<std::chrono::duration<double>> timings, std::string name);
};

class CombinationGen {
public:
    CombinationGen(uint64_t min, uint64_t max, size_t k);
    std::optional<std::vector<uint64_t>> next();
private:
    private:
    int min_;
    int max_;
    size_t K;
    size_t N;
    std::vector<size_t> indices;
    bool done;

    void advance();
};

#endif