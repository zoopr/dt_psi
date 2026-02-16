#ifndef PROTO_RECONSTRUCTOR
#define PROTO_RECONSTRUCTOR

#include <vector>
#include <array>
#include <set>

#include "crypto/crypto_types.h"
#include "protocol/keyholder.h"

extern "C" {
    #include "sss.h"
}

class Reconstructor{
private:
    reconstructor_proto_data_t params;
    uint64_t current_round = 0;
    std::vector<std::vector<cpp_share>> current_share_table;
    std::set<uint64_t> current_psi;    
public:
    bool init_data(KeyHolder* kh);

    bool decrypt_row(uint8_t* in, size_t in_len);// TODO Ciphertext data structure
    
    void reconstruct_round(); // TODO combinatory subgroups + SSS reconstruction on known value.
};

#endif