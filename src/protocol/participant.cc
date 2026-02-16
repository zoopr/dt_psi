#include <algorithm>
#include <iterator>
#include <vector>
#include <cstring>


#include "crypto/crypto_primitives.h"
#include "participant.h"

bool Participant::init_data(KeyHolder *kh)
{
    return kh->serve_participant(&params);
}

void Participant::explore(uint64_t steps)
{
    // Simple random addition. TODO More complex strategies based on map morphology.
    for(uint64_t i = 0; i<steps; ++i) {
        explored.insert(CryptoPrimitives::secure_random_uint64(params.coord_range));
    }
}

bool Participant::send_round_shares(Reconstructor *r)
{
    // Construct Ei from edge subset (explored coordinates which weren't part of last PSI)
    std::set<uint64_t> frontier;
    std::set_difference(
        explored.begin(), explored.end(),
        confirmed.begin(), confirmed.end(),
        std::inserter(frontier,frontier.begin())
    );
    cpp_share tmp;
    std::vector<cpp_share> ei(params.coord_range);
    for (uint64_t i = 0; i < params.coord_range; i++) {
        if (frontier.find(i) != frontier.end()) {
            // If the index is in frontier, set legitimate share.
            ei[i] = params.shareMatrix[current_round*params.coord_range + i];
        } else {
            // Generate independent dummy from arbitrary degree-1 polynomial.
            CryptoPrimitives::gen_dummy(tmp.data(),sizeof(tmp));
            ei[i] = tmp;
        }
    }

    // KEM+AEAD encryption.
    uint8_t eph_pr[32],eph_pub[32],ss[32],roundkey[32];
    // Create ephemeral pair
    CryptoPrimitives::x25519_keypair(eph_pub,eph_pr);
    // Shared secret from eph private and Reconstructor pub
    CryptoPrimitives::x25519_shared(ss,eph_pr,params.reconstructor_pubkey);
    // Create symmetric round key with info = little-endian round value
    CryptoPrimitives::hkdf_sha256(roundkey,32,ss,32,0,0,(const uint8_t *)&current_round,sizeof(current_round));

    std::vector<uint8_t> c_sym(sizeof(ei));
    std::vector<uint8_t> produced_nonce(12); // Generated inside encryption, not here!
    // Not great cast to ei[0][0] for start pointer. they SHOULD be continuously allocated.
    CryptoPrimitives::aes_gcm_encrypt(c_sym.data(), sizeof(c_sym),ei.data()->data(),sizeof(ei),(const uint8_t *)&current_round,sizeof(current_round),roundkey,produced_nonce.data());
    
    // Generate final ciphertext to MAC
    std::vector<uint8_t> C_i(sizeof(eph_pub)+sizeof(produced_nonce)+sizeof(c_sym)+sizeof(current_round));
    // TERRIBLE pointer math to allocate continuously.
    uint8_t* ci_ptr = C_i.data();
    std::memcpy(ci_ptr,eph_pub,sizeof(eph_pub));
    ci_ptr += sizeof(eph_pub);
    std::memcpy(ci_ptr,produced_nonce.data(),sizeof(produced_nonce));
    ci_ptr += sizeof(produced_nonce);
    std::memcpy(ci_ptr,c_sym.data(),sizeof(c_sym));
    ci_ptr += sizeof(c_sym);
    std::memcpy(ci_ptr,&current_round,sizeof(current_round));

    // Derive round-MAC key. NOTE: reuse roundkey buffer for simplicity. Be careful.
    CryptoPrimitives::hkdf_sha256(roundkey,32,params.kG,32,0,0,(const uint8_t *)&current_round,sizeof(current_round));
    std::array<uint8_t, 64> hmac_tag; // Could be 32, but we are forcing compatibility with Sha512 if we want to use it.
    size_t tag_len = 64;
    CryptoPrimitives::aes_hmac_tag(hmac_tag.data(),&tag_len,roundkey,sizeof(roundkey),C_i.data(),sizeof(C_i));

    // Final structure to send: (C_i || tag, sizeof()) 
    std::vector<uint8_t> final_c(sizeof(C_i)+tag_len);
    std::memcpy(final_c.data(),C_i.data(),sizeof(C_i));
    std::memcpy(final_c.data()+sizeof(C_i),hmac_tag.data(),tag_len);

    r->decrypt_row(final_c.data(), sizeof(final_c));
    return true;
}
