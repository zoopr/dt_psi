#ifndef CRYPTO_TYPES
#define CRYPTO_TYPES


#include <cstddef>
#include <cstdint>
#include <vector>

extern "C" {
    #include <sss.h>
}

typedef std::array<uint8_t, sss_SHARE_LEN> cpp_share;

typedef struct {
    uint64_t coord_range; 
    uint8_t max_parties;
    uint8_t max_rounds; 
    uint8_t threshold;
    std::vector<cpp_share> shareMatrix;
    uint8_t kG[32];
    uint8_t reconstructor_pubkey[32];
} participant_proto_data_t;

typedef struct {
    uint64_t coord_range; 
    uint8_t max_parties;
    uint8_t max_rounds; 
    uint8_t threshold;
    uint8_t kG[32];
    uint8_t reconstructor_privkey[32];
} reconstructor_proto_data_t;

#endif