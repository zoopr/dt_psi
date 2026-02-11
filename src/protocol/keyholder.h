#ifndef PROTO_KH
#define PROTO_KH

#include <cstddef>
#include <cstdint>
#include <vector>

extern "C" {
    #include <sss.h>
}

#define KG_LEN 64
#define REC_KEY_LEN 64


class KeyHolder {
private:
    std::vector<sss_Share> polymatrix;
    bool isInitialized = false;
    uint8_t kg[KG_LEN];
    uint8_t r_priv[REC_KEY_LEN], r_pub[REC_KEY_LEN];    

public:
    KeyHolder(const uint64_t coords, const uint8_t max_parties, const uint8_t max_rounds, const uint8_t threshold);

    bool serve_participant();

    bool serve_reconstructor();
};

#endif