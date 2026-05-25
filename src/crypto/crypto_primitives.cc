#include "crypto_primitives.h"

#include <stdexcept>
#include <iostream>
#include <cmath>
#include <numeric>

#include <openssl/aead.h>
#include <openssl/curve25519.h>
#include <openssl/hkdf.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/crypto.h>



extern "C" {
#include <sss.h>
#include <cstring>
}




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

bool CryptoPrimitives::aes_gcm_encrypt(std::vector<uint8_t>* out, size_t out_len, const uint8_t *plaintext, size_t plaintext_len,
                                       const uint8_t *aad, size_t aad_len, const uint8_t *key, uint8_t *nonce) {
    const EVP_AEAD* aead = EVP_aead_aes_256_gcm();

    EVP_AEAD_CTX ctx;
    if (!EVP_AEAD_CTX_init(&ctx, aead, key, 32, 16, nullptr)) {
        return false;
    }

    // We generate the nonce ourselves on the external buffer.
    // Note: we can't check that the buffer is actually 96 bits. 
    // That's a problem for the participant.
    // We really should be using a determninistic counter instead, 
    // but since we only use ephemeral HKDF-based keys this is still safe.
    RAND_bytes(nonce, 12);

    size_t max_out_len = plaintext_len + EVP_AEAD_max_overhead(aead);
    // Check that our output buffer size (set to its actual max size) is smaller than this
    if (max_out_len > out_len){
        // std::cout << "Need to resize to MAX_OUT_LEN: "<< max_out_len <<std::endl;
        out->resize(max_out_len);
    }

    if (!EVP_AEAD_CTX_seal(
            &ctx,
            out->data(),
            &out_len,
            max_out_len,
            nonce,
            12,
            plaintext,
            plaintext_len,
            aad,
            aad_len)) {
        EVP_AEAD_CTX_cleanup(&ctx);
        return false;
    }
    out->resize(out_len); // Crop down to ciphertext.

    EVP_AEAD_CTX_cleanup(&ctx);
    return true;
}

bool CryptoPrimitives::aes_gcm_decrypt(std::vector<uint8_t>* out, size_t out_len, const uint8_t *ciphertext, size_t ciphertext_len,
                                       const uint8_t *aad, size_t aad_len, const uint8_t *key, const uint8_t *nonce) {
    const EVP_AEAD* aead = EVP_aead_aes_256_gcm();

    EVP_AEAD_CTX ctx;
    if (!EVP_AEAD_CTX_init(&ctx, aead, key, 32, 16, nullptr)) {
        return false;
    }

    // Same as encryption. Check structure for max allowed size, then write down actual size.
    if (out_len < ciphertext_len){
        out->resize(ciphertext_len);
    }

    if (!EVP_AEAD_CTX_open(
            &ctx,
            out->data(),
            &out_len,
            ciphertext_len,
            nonce,
            12,
            ciphertext,
            ciphertext_len,
            aad,
            aad_len)) {
        EVP_AEAD_CTX_cleanup(&ctx);
        return false;  // authentication failure
    }
    out->resize(out_len); // Crop down to plaintext.

    EVP_AEAD_CTX_cleanup(&ctx);
    return true;
}

bool CryptoPrimitives::aes_hmac_tag(uint8_t *out, size_t *out_len, const uint8_t *key, const size_t key_len, const uint8_t *data, const size_t data_len)
{
    if (*out_len < EVP_MAX_MD_SIZE){
        throw std::runtime_error("Error generating HMAC tag: no guarantee of buffer size!");
    }

    HMAC(EVP_sha256(),key,key_len,data,data_len,out,(unsigned int*)out_len);
    return true;
}

bool CryptoPrimitives::sss_share_gen(uint8_t *out, size_t out_len, uint8_t *data, const uint8_t num_shares, const uint8_t threshold){
    // SSS Share generation code.
    if (out_len < sizeof(sss_Share)*num_shares) return false; // We do not have enough space to store the required shares!
    // Data should be a C-string, but we aren't checking that.
    sss_Share* out_shares = (sss_Share*) out;
    sss_create_shares(out_shares, data, num_shares, threshold);
    return true;
}

bool CryptoPrimitives::sss_share_reconstruct(uint8_t *out, size_t out_len, uint8_t *in, size_t in_len, const uint8_t num_shares){
    // SSS Share reconstruction code.
    uint8_t outbuf[sss_MLEN];
    
    sss_combine_shares(outbuf,(sss_Share*) in, num_shares);
    memcpy(out,outbuf,out_len);
    return true;
}

uint64_t CryptoPrimitives::secure_random_uint64(const uint64_t max)
{
    if (max == 0) return 0;

    uint64_t x;
    uint64_t limit = UINT64_MAX - (UINT64_MAX % max);

    do {
        RAND_bytes(reinterpret_cast<uint8_t*>(&x), sizeof(x));
    } while (x >= limit);

    return x % max;
}

void CryptoPrimitives::gen_dummy(uint8_t *out, size_t out_len)
{
    sss_create_shares((sss_Share*)out, (const uint8_t*)"dummy", 1, 1);

}

bool CryptoPrimitives::secure_compare(uint8_t *b1, uint8_t *b2, size_t len)
{
    // Returns true on exact match up to len.
    return !CRYPTO_memcmp(b1, b2, len);
}

void CryptoPrimitives::print_stats(std::vector<std::chrono::duration<double>> timings, std::string name)
{
    using Duration = std::chrono::duration<double>;

    Duration mean,var;

    if (timings.empty()) {
        mean = Duration{0};
        var = Duration{0};
    } else {
        Duration sum = std::reduce(timings.begin(), timings.end());
        mean = sum / timings.size();
        var = Duration{std::accumulate(timings.begin(), timings.end(), 0.0, [mean](double acc, const Duration& s){return acc + (std::pow((s-mean).count(),2));})}/ timings.size();
    }

    std::cout << name << " average:" << mean.count() <<std::endl;
    std::cout << name << " variance:" << var.count() <<std::endl;
}

CombinationGen::CombinationGen(uint64_t min, uint64_t max, size_t k)
: min_(min), max_(max), K(k),
          N(max - min + 1),
          indices(k),
          done(false)
{
    if (K > N || K == 0) {
        done = true;
        return;
    }

    for (size_t i = 0; i < K; ++i)
        indices[i] = i;
}


std::optional<std::vector<uint64_t>> CombinationGen::next()
{
    if (done) return std::nullopt;

        std::vector<uint64_t> result;
        result.reserve(K);

        for (size_t i = 0; i < K; ++i)
            result.push_back(min_ + indices[i]);

        advance();
        return result;
}

void CombinationGen::advance()
{
    int i = K - 1;

    while (i >= 0 && indices[i] == N - K + i)
        --i;

    if (i < 0) {
        done = true;
        return;
    }

    ++indices[i];

    for (size_t j = i + 1; j < K; ++j)
        indices[j] = indices[j - 1] + 1;
}
