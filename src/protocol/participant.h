#ifndef PROTO_PARTICIPANT
#define PROTO_PARTICIPANT

#include <set>
#include <chrono>

#include "crypto/crypto_types.h"
#include "protocol/keyholder.h"
#include "protocol/reconstructor.h"

class Participant{
private:
    participant_proto_data_t params;
    uint64_t current_round = 0;
    std::set<uint64_t> explored;
    std::set<uint64_t> confirmed;
    inline static std::vector<std::chrono::duration<double>> row_enc_timings{};

public:
    bool init_data(KeyHolder* kh);
    void explore(uint64_t steps);
    bool send_round_shares(Reconstructor* r);
    void update_confirmed(std::set<uint64_t> last_PSI);
    void print_stats();
};

#endif