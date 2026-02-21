#ifndef PROTO_KH
#define PROTO_KH

#include <cstddef>
#include <cstdint>
#include <vector>
#include <array>
#include <chrono>

extern "C" {
    #include <sss.h>
}

#include "crypto/crypto_types.h"

#define KG_LEN 32
#define REC_KEY_LEN 32
#define SALT_LEN 32


class KeyHolder {
private:
    uint64_t max_coords;
    uint8_t max_parties, max_rounds, threshold;

    std::vector<cpp_share> polymatrix;
    bool isInitialized = false;
    uint8_t kg[KG_LEN];
    uint8_t r_priv[REC_KEY_LEN], r_pub[REC_KEY_LEN];
    uint8_t common_salt[SALT_LEN] = {0,}; 
    uint8_t last_served;

    std::vector<std::chrono::duration<double>> proto_init_timings;
public:
    KeyHolder(const uint64_t coords, const uint8_t max_parties, const uint8_t max_rounds, const uint8_t threshold);

    bool serve_participant(participant_proto_data_t* in);

    bool serve_reconstructor(reconstructor_proto_data_t* in);

    void print_stats();
};

#endif