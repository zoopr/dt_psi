#include <string.h>
#include <cstdio>
#include <stdexcept>


#include <openssl/rand.h>

#include "crypto/crypto_primitives.h"
#include "keyholder.h"

KeyHolder::KeyHolder(const uint64_t coords, const uint8_t max_parties, const uint8_t max_rounds, const uint8_t threshold)
:     polymatrix(max_rounds*coords*max_parties)
{
    // Initialize internal structures for operation

    // Create coords * max_rounds polynomials, generating shares via CryptoPrimitives::sss_gen()
    // Save this matrix in polymatrix. Serve one vector of these to each participant.

    char msgbuf[sss_MLEN] = {0,};

    for (uint8_t round = 0; round < max_rounds; round++) {
        uint64_t poly_round_i = round * coords * max_parties;
        for (uint64_t coord=0; coord < coords; coord++) {
            uint64_t poly_coord_i = coord * max_parties;
            std::snprintf(msgbuf,sss_MLEN,"%d,%d",round,coord);
            CryptoPrimitives::sss_share_gen(polymatrix[poly_round_i+poly_coord_i],sizeof(sss_Share)*max_parties, (uint8_t*)msgbuf, max_parties, threshold);
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

    // Setup KDF through common parameters. Maybe communicate those too. Maybe they're already hardcoded for everyone. We'll see.
}

bool KeyHolder::serve_participant()
{
    // Collect and write into participant buffers (TODO function signature) personal share matrix, kG, KDF params, pubkey of R.
    return false;
}

bool KeyHolder::serve_reconstructor()
{
    // Collect and write into reconstructor buffers (TODO function signature) kG, KDF parameters, privkey of R.
    return false;
}
