
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <stdexcept>


#include <sss.h>


#include <crypto/crypto_primitives.h>
#include <protocol/keyholder.h>
#include <protocol/participant.h>
#include <protocol/reconstructor.h>


// // Toy example macros, leave when done
// #define NUM_SHARES 5
// #define THRESHOLD 3

void base_experiment(uint8_t num_participants, uint64_t coords_size, uint8_t max_rounds, uint8_t threshold){
        /*
    Objectives: 
    - Offer high-level overview and metrics on total map coverage
    - Offer data to create informed strategies for independent coverage of the same area
    - Protect current location and location history of each individual drone, from each other and from the central authority.    
    */  

    /*
    PHASE 1: Initialization
    KeyHolder MUST:
    - Create (num_rounds * num_coords) polynomials of degree (threshold-1). Value of degree 0 factor must be (coordinate).
    - Create Kg generator key for round keys via independent KDF. Set KDF parameters to confirm with participants.
    - Generate public and private key for communication between participants and the reconstructor (pkR, skR)

    When asked for shares by participant Pi:
    - Create share matrix. Random points (no consistent x! Very important) for each polynomial. Total is obviously (num_rounds * num_coords)
    - Send this share matrix, Kg, the KDF parameters, and pkR.

    When asked for data by reconstructor R:
    - Send Kg, KDF params, skR.

    After initial dispatch phase is over, KH may destroy all data, including polynomials.

    */
    KeyHolder kh(coords_size,num_participants,max_rounds,threshold);
    kh.print_stats();

    std::vector<Participant> partList;
    for (int i = 0; i < num_participants; ++i) {
        Participant p;
        p.init_data(&kh);
        partList.push_back(p);
    }


    Reconstructor r;
    r.init_data(&kh);

    /*
    PHASE 2: Exploration and reconstruction
    Each participant Pi has a private set EXPLORED which contains which coordinates it has personally explored.
    Each round r between 0 and num_rounds-1 :
    - Create conditional share vector based on the share matrix row for round r. Each element is either the share if coordinate j is in (EXPLORED - last PSI), or a dummy value if not.
    - Encrypt this share vector using KEM+AEAD. Follow protocol description. Implementation is very sensitive and should be kept inside participant.cc
    - Send HMAC-tagged ciphertext to the reconstructor.

    The reconstructor, after receiving one of these ciphertexts:
    - Decrypts the ciphertext based on the skR and the symmetric keys based on KDF, kG and the KEM system.
    - Adds the row to a big share matrix.

    After a time limit or after receiving all rows. the reconstructor:
    - Creates all permutations of (t over n) points for a specific coordinate index and performs Lagrange interpolation. 
    - If it decrypts for (round,coordinate), then that is a valid collection of shares and the coordinate is over threshold.
    - Add point to final PSI, move to next coordinate.
    */

    // Keep in mind fully random exploration per-step != 100 distinct coordinates!
    // But we can create a partially informed local exploration strategy by prioritizing coordinates which are still under threshold.

    for (int round = 0; round < max_rounds; ++round){
        for (int i = 0; i < num_participants; ++i) {
            partList[i].explore(1);
            partList[i].send_round_shares(&r);
        }
        r.reconstruct_round();
        
        std::cout << "Round " << round << " PSI size="<<r.get_psi().size()<<std::endl;
    /*
    PHASE 3: HANDOVER
    After completing the PSI evaluation, the reconstructor:
    - Excludes these coordinates from future PSI tests! We are already over threshold for these and the set is strictly growing.
    - Send PSI in a broadcast encrypted with skR.

    When receiving the PSI broadcast, each participant:
    - Creates the TO_EXPLORE coordinate subset by intersecting complement of EXPLORED with complement of PSI.
    - Updates a different subset DONOT_SHARE which is the union of TO_EXPLORE and PSI. Essentially, sharing already confirmed coords opens us to differential attacks.
    - Based on current location, select likely candidates to explore during next round. This can be local exploration, or any other strategy.
    - This strategy is developed independently by each member.
    */
        for (int i = 0; i < num_participants; ++i) {
            partList[i].update_confirmed(r.get_psi());
        }

        // Print stats.
        partList[0].print_stats();
        r.print_stats();

    }
}

int main(int argc, char *argv[]){

    // Toy example: are our primitives imported properly

    // uint8_t priv[32];
    // uint8_t pub[32];


    // sss_Share ourShares[NUM_SHARES];
    // const char secret[sss_MLEN] = "wow this works";
    // char reconstruct[sss_MLEN];

    // // Curve 25519 tests

    // CryptoPrimitives::x25519_keypair(priv,pub);

    // std::cout << "Priv: " << std::endl;
    // for (const auto& e : priv) {
    //     std::cout << std::hex << +e;
    // }

    // std::cout << "\nPub: " << std::endl;
    // for (const auto& e : pub) {
    //     std::cout << std::hex << +e;
    // }

    // // SSS tests
    // // Construct N shares at threshold T
    // CryptoPrimitives::sss_share_gen(*ourShares, sizeof(ourShares), (uint8_t*) secret, (const uint8_t) NUM_SHARES, (const uint8_t) THRESHOLD); 
    // // Reconstruct from shares 3-5. Obviously the shares size is wrong now, but the primitive doesn't use it.
    // CryptoPrimitives::sss_share_reconstruct((uint8_t*) reconstruct, sizeof(reconstruct), ourShares[2], sizeof(ourShares), (const uint8_t) THRESHOLD);

    // std::cout << "\nSSS test....\nReconstructed:" << reconstruct << std::endl;


    // TODO Protocol flow.
    try { 

    uint8_t max_rounds = 10; // No real point variating this except for demonstrating short circuit efficiency in low convergence scenarios.

    for (uint8_t threshold = 2; threshold < 5; threshold++){
        for (uint64_t coords_size = 100; coords_size < 10000; coords_size *= 10){
            for (int num_participants = 3; num_participants < 256; num_participants = (num_participants+1) *2 - 1) { // We want to test up to 255 but for condition gets funky around int limit. Easier like this.
                std::cout<<"Starting experiment N="<<num_participants<<",L="<<coords_size<<",Rmax="<<(int)max_rounds<<",T="<<(int)threshold<<std::endl;
                base_experiment((uint8_t)num_participants, coords_size, max_rounds, threshold);
            }
        }
    }        
        

    } catch (const std::exception& e) {
        std::cerr << "Main test failed: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
