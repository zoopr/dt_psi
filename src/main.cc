
#include <cstddef>
#include <cstdint>
#include <iostream>


#include <crypto/crypto_primitives.h>

int main(int argc, char *argv[]){

    // Toy example: are our primitives imported properly

    // uint8_t priv[32];
    // uint8_t pub[32];

    // CryptoPrimitives::x25519_keypair(priv,pub);

    // std::cout << "Priv: " << std::endl;
    // for (const auto& e : priv) {
    //     std::cout << std::hex << +e;
    // }

    // std::cout << "\nPub: " << std::endl;
    // for (const auto& e : pub) {
    //     std::cout << std::hex << +e;
    // }

    // TODO Protocol flow.
    
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

    /*
    PHASE 2: Exploration and reconstruction
    Each participant Pi has a private set EXPLORED which contains which coordinates it has personally explored.
    Each round r between 0 and num_rounds-1 :
    - Create conditional share vector based on the share matrix row for round r. Each element is either the share if coordinate j is in EXPLORED, or a uniformly random value if not.
    - Encrypt this share vector using KEM+AEAD. Follow protocol description. Implementation is very sensitive and should be kept inside participant.cc
    - Send GMAC-tagged ciphertext to the reconstructor.

    The reconstructor, after receiving one of these ciphertexts:
    - Decrypts the ciphertext based on the skR and the symmetric keys based on KDF, kG and the KEM system.
    - Adds the row to a big share matrix.

    After a time limit or after receiving all rows. the reconstructor:
    - Creates all permutations of (t over n) points for a specific coordinate index and performs Lagrange interpolation. 
    - If it decrypts for (coordinate), then that is a valid collection of shares and the coordinate is over threshold.
    - Add point to final PSI, move to next coordinate.
    */

    /*
    PHASE 3: HANDOVER
    After completing the PSI evaluation, the reconstructor:
    - Excludes these coordinates from future PSI tests! We are already over threshold for these and the set is strictly growing.
    - Send PSI in a broadcast encrypted with skR.

    When receiving the PSI broadcast, each participant:
    - Creates the TO_EXPLORE coordinate subset by intersecting complement of EXPLORED with complement of PSI.
    - Based on current location, select likely candidates to explore during next round. This can be local exploration, or any other strategy.
    - This strategy is developed independently by each member.
    */

    return 0;
}
