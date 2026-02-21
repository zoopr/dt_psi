#include <algorithm>
#include <iterator>
#include <vector>
#include <cstring>
#include <iostream>
#include <numeric>


#include "crypto/crypto_primitives.h"
#include "participant.h"

bool Participant::init_data(KeyHolder *kh)
{
    return kh->serve_participant(&params);
}

void Participant::explore(uint64_t steps)
{
    if (steps <= 0) return;
    std::set<uint64_t> not_needed;
    std::set_union(explored.begin(), explored.end(),
        confirmed.begin(), confirmed.end(),
        std::inserter(not_needed,not_needed.begin()));
    // Simple rejection sampling. This slows down the sim in "real" scenarios. 
    // Better to sample from the complementary set.
    // For now this is ok, because we're not timing the actual exploration algorithms.
    if (not_needed.size() == params.coord_range){
        // std::cout << "Nothing left to explore! Rest of the algo should be short circuiting." <<std::endl;
        return;
    }
    uint64_t cand;
    do {
        cand = CryptoPrimitives::secure_random_uint64(params.coord_range);
        // std::cout << "DEBUG: precise cand cnt: " <<explored.count(cand)<<","<< confirmed.count(cand)<<std::endl;           
    }
    while(not_needed.count(cand));
    explored.insert(cand);
    // std::cout << "DEBUG inserted "<<cand<<"into participant."<<std::endl;
    return explore(steps-1);
}

bool Participant::send_round_shares(Reconstructor *r)
{
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

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
            // std::cout << "Putting real share for coord "<< std::dec << i << std::endl;
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
    // std::cout << "DEBUG roundkey par: " << roundkey << std::endl;


    std::vector<uint8_t> c_sym;
    std::array<uint8_t, 12> produced_nonce; // Generated inside encryption, not here!
    // Not great cast to ei[0][0] for start pointer. they SHOULD be continuously allocated.
    CryptoPrimitives::aes_gcm_encrypt(&c_sym, 0,ei.data()->data(),ei.size()*sizeof(ei[0]),(const uint8_t *)&current_round,sizeof(current_round),roundkey,produced_nonce.data());
    
    // Generate final ciphertext to MAC
    // std::cout << "Generate final ciphertext to MAC" << std::endl;
    std::vector<uint8_t> C_i(sizeof(eph_pub)+12+(c_sym.size())+sizeof(current_round)); // C_sym has size()*uint8_t=1 size
    // TERRIBLE pointer math to allocate continuously.
    uint8_t* ci_ptr = C_i.data();
    std::memcpy(ci_ptr,eph_pub,sizeof(eph_pub));
    ci_ptr += sizeof(eph_pub);
    std::memcpy(ci_ptr,produced_nonce.data(),12);
    ci_ptr += 12;
    std::memcpy(ci_ptr,c_sym.data(),c_sym.size());
    ci_ptr += c_sym.size();
    std::memcpy(ci_ptr,&current_round,sizeof(current_round));

    // Derive round-MAC key. NOTE: reuse roundkey buffer for simplicity. Be careful.
    
    // std::cout << "Derive MAC key" << std::endl;
    CryptoPrimitives::hkdf_sha256(roundkey,32,params.kG,32,0,0,(const uint8_t *)&current_round,sizeof(current_round));
    std::array<uint8_t, 64> hmac_tag; // Could be 32, but we are forcing compatibility with Sha512 if we want to use it.
    size_t tag_len = 64;
    CryptoPrimitives::aes_hmac_tag(hmac_tag.data(),&tag_len,roundkey,sizeof(roundkey),C_i.data(),sizeof(C_i));

    // Final structure to send: (C_i || tag, sizeof()) 
    
    std::vector<uint8_t> final_c(C_i.size()+tag_len);
    std::memcpy(final_c.data(),C_i.data(),C_i.size());
    if (tag_len != 32) {
        throw std::runtime_error("Tag_len != 32 but we should be in SHA256. Wrong params?");
    }
    std::memcpy(final_c.data()+C_i.size(),hmac_tag.data(),32);

    
    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    row_enc_timings.push_back(std::chrono::duration_cast<std::chrono::duration<double>>(end-start));

    r->decrypt_row(final_c.data(), final_c.size());

    current_round += 1;

    return true;
}

void Participant::update_confirmed(std::set<uint64_t> last_PSI)
{
    if (last_PSI.size())
        confirmed = last_PSI;
}

void Participant::print_stats()
{
    // Row encryption stats
    
    auto const count = static_cast<float>(row_enc_timings.size());
    std::cout << "Row encryption average:" << ((std::reduce(row_enc_timings.begin(), row_enc_timings.end())) / count).count() <<std::endl;
    row_enc_timings.clear();
}
