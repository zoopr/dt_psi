#include <string.h>
#include <cstdio>
#include <stdexcept>
#include <cstring>
#include <iostream>
#include <numeric>

#include <openssl/rand.h>

#include "crypto/crypto_primitives.h"
#include "keyholder.h"

KeyHolder::KeyHolder(const uint64_t coords, const uint8_t max_parties, const uint8_t max_rounds, const uint8_t threshold)
:     polymatrix(max_rounds*coords*max_parties), last_served(max_parties), max_coords(coords), max_parties(max_parties), max_rounds(max_rounds), threshold(threshold)
{
    std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

    // Initialize internal structures for operation

    // Create coords * max_rounds polynomials, generating shares via CryptoPrimitives::sss_gen()
    // Save this matrix in polymatrix. Serve one vector of these to each participant.

    char msgbuf[sss_MLEN] = {0,};

    for (uint8_t round = 0; round < max_rounds; round++) {
        uint64_t poly_round_i = round * coords * max_parties;
        for (uint64_t coord=0; coord < coords; coord++) {
            uint64_t poly_coord_i = coord * max_parties;
            std::snprintf(msgbuf,sss_MLEN,"%d,%d",round,coord);
            CryptoPrimitives::sss_share_gen(polymatrix[poly_round_i+poly_coord_i].data(),sizeof(sss_Share)*max_parties, 
                                                (uint8_t*)msgbuf, max_parties, threshold);
        }
    }

    // Generate kG via source of randomness of openSSL/boringSSL
    if (RAND_bytes(kg, KG_LEN) != 1) {
        throw std::runtime_error("Error generating kG. exiting...\n");
    }
    // printf("Generated key: ");
    // for (size_t i = 0; i < KG_LEN; i++)
    //     printf("%02x", kg[i]);
    // printf("\n");

    // Generate public-private key pair for the reconstructor. X25519 ok?
    CryptoPrimitives::x25519_keypair(r_pub, r_priv);

    std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
    proto_init_timings.push_back(std::chrono::duration_cast<std::chrono::duration<double>>(end-start));
}

bool KeyHolder::serve_participant(participant_proto_data_t *in)
{
    
    // Serve maximum of N parties
    if (last_served == 0){
        throw std::runtime_error("Already served N parties, we ran out of shares!");
    }

    in->coord_range = max_coords; 
    in->max_parties = max_parties;
    in->max_rounds = max_rounds; 
    in->threshold = threshold;
    std::memcpy(in->kG,kg,KG_LEN);
    std::memcpy(in->reconstructor_pubkey,r_pub,REC_KEY_LEN);

    // Warning: share matrix should already be initialized. But we will reallocate it to the appropriate size now.
    in->shareMatrix.resize(max_rounds * max_coords);

    for (uint8_t round = 0; round < max_rounds; round++) {
        uint64_t poly_round_i = round * max_coords;
        for (uint64_t coord=0; coord < max_coords; coord++) {
            uint64_t vector_i = poly_round_i +  coord;
            // Create different indices between the 3-dimensional (R x J x N) polymatrix and the (R x J x 1) participant's share matrix
            uint64_t poly_base = (vector_i )* max_parties;
            uint64_t idx = CryptoPrimitives::secure_random_uint64(last_served);
            // Switch chosen element to tail element, serve tail element, discard from future use.
            std::swap(polymatrix[poly_base+idx],polymatrix[poly_base+last_served-1]);
            in->shareMatrix[vector_i] = polymatrix[poly_base+last_served-1];
        }
    }
    --last_served;
    // std::cout << "Served one participant." << std::endl; 
    return true;
}

bool KeyHolder::serve_reconstructor(reconstructor_proto_data_t *in)
{

    in->coord_range = max_coords; 
    in->max_parties = max_parties;
    in->max_rounds = max_rounds; 
    in->threshold = threshold;
    std::memcpy(in->kG,kg,KG_LEN);
    std::memcpy(in->reconstructor_privkey,r_priv,REC_KEY_LEN);
    return true;
}

void KeyHolder::print_stats()
{
    CryptoPrimitives::print_stats(proto_init_timings, "Protocol initialization");

    proto_init_timings.clear();
}
