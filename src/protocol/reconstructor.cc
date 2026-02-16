#include "crypto/crypto_primitives.h"
#include "reconstructor.h"

bool Reconstructor::init_data(KeyHolder *kh)
{
    return kh->serve_reconstructor(&params);
}

bool Reconstructor::decrypt_row(uint8_t *in, size_t in_len)
{
    // TODO
    return false;
}
