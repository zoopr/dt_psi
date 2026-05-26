
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <numeric>

#include "crypto/crypto_primitives.h"
#include "reconstructor.h"

bool Reconstructor::init_data(KeyHolder *kh)
{
    return kh->serve_reconstructor(&params);
}

bool Reconstructor::decrypt_row(uint8_t *in, size_t in_len)
{
    std::chrono::high_resolution_clock::time_point start, end;
    // See paper protocol for precise implementation details.
    
    std::array<uint8_t, 32> reported_tag; // Could be 32, but we are forcing compatibility with Sha512 if we want to use it.
    std::array<uint8_t, 64> hmac_tag; // Could be 32, but we are forcing compatibility with Sha512 if we want to use it.
    size_t tag_len = 32; // Hardcoded because of our use of sha256. TODO: move to KH-defined params as in the paper!
    
    size_t Ci_len = in_len-tag_len;
    std::vector<uint8_t> C_i(Ci_len);    
    
    std::array<uint8_t, 12> produced_nonce;

    uint8_t mac_key[32];
    uint8_t eph_pub[32],ss[32],roundkey[32];
    uint64_t current_round_received;

    // Only the (slightly) variable size ciphertext is left.
    size_t csym_size = Ci_len-(sizeof(eph_pub)+produced_nonce.size()+sizeof(current_round_received));
    size_t csym_off = sizeof(eph_pub)+produced_nonce.size();
    std::vector<uint8_t> C_sym(csym_size);

    std::vector<uint8_t> raw_ei(params.coord_range*sizeof(cpp_share));
    std::vector<cpp_share> ei(params.coord_range);



    start = std::chrono::high_resolution_clock::now();


    // Generate MAC key for tag retrieval.
    
    // std::cout << "Generate Reconstructor mac" << std::endl;
    CryptoPrimitives::hkdf_sha256(mac_key,32,params.kG,32,0,0,(const uint8_t *)&current_round,sizeof(current_round));
    
    // Dissect ciphertext.
    
    // std::cout << "Dissect ciphertext" << std::endl;

    // std::cout << "Expected ciphertext len: " <<Ci_len<< std::endl;
    std::memcpy(C_i.data(), in, Ci_len);
    std::memcpy(reported_tag.data(), in+Ci_len, tag_len);
    
    // Confirm HMAC
    
    // std::cout << "Confirm HMAC" << std::endl;
    tag_len = 64; // For compatibility with max allowed SHA512. We will use Sha256, and this is reflected in dissection above.
    CryptoPrimitives::aes_hmac_tag(hmac_tag.data(),&tag_len,mac_key,sizeof(mac_key),C_i.data(),sizeof(C_i));

    if (tag_len != 32){
        throw std::runtime_error("Incorrect HMAC tag length. Wrong params?");
    } else if (!CryptoPrimitives::secure_compare(reported_tag.data(),hmac_tag.data(),tag_len)){ // Non-critical error, but wrong params!
        return false;
    }
    // Determine round key from given eph_pub + r_priv
    // KEM+AEAD encryption.
    // Ephemeral pubkey is at offset 0.
    std::memcpy(eph_pub, C_i.data(), 32);
    // Shared secret from eph private and Reconstructor pub
    CryptoPrimitives::x25519_shared(ss,params.reconstructor_privkey,eph_pub);
    // Create symmetric round key with info = little-endian round value
    CryptoPrimitives::hkdf_sha256(roundkey,32,ss,32,0,0,(const uint8_t *)&current_round,sizeof(current_round));
    // std::cout << "DEBUG roundkey rec: " << roundkey << std::endl;
    
    
    // Nonce is second.
    std::memcpy(produced_nonce.data(), C_i.data()+sizeof(eph_pub), 12);

    std::memcpy(&current_round_received, C_i.data()+C_i.size()-sizeof(current_round_received), sizeof(current_round_received));
    // Check if we are synchronized!
    if (current_round_received != current_round) {
        // std::cout << "Discrepancy in round numbers: "<< current_round_received << "vs. local " << current_round << ". Should we correct this?" << std::endl;
        return false;// TODO wipe matrix!
    }


    // Third member in C_i bufer.
    std::memcpy(C_sym.data(), C_i.data()+csym_off, csym_size);
    
    // Decrypt C_i based on received nonce and round key
    if (!CryptoPrimitives::aes_gcm_decrypt(&raw_ei,raw_ei.size()*sizeof(raw_ei[0]),C_sym.data(),csym_size,(const uint8_t *)&current_round,sizeof(current_round),roundkey,produced_nonce.data())){
        // std::cout << "Decryption unsuccessful! Skipping row..."<<std::endl;
        return false;
    }
    // Just for convenience, copy into proper vector of shares, then add to matrix.
    std::memcpy(ei.data()->data(),raw_ei.data(),params.coord_range*sizeof(cpp_share));
    
    end = std::chrono::high_resolution_clock::now();
    row_dec_timings.push_back(std::chrono::duration_cast<std::chrono::duration<double>>(end-start));

    std::cout << "Row decryption complete. Duration: " << std::chrono::duration_cast<std::chrono::duration<double>>(end-start).count() <<"s "<<std::endl;

    current_share_table.push_back(ei);
    // std::cout << "Decryption successful. Current Reconstructor rows: "<< current_share_table.size() << std::endl;

    return true;
}

void Reconstructor::reconstruct_round()
{
    // TODO
    // Process vector of decrypted rows.
    // For each coordinate not in the confirmed PSI:
    uint8_t msgbuf[sss_MLEN] = {0,};
    uint8_t outbuf[sss_MLEN] = {0,};
    std::chrono::high_resolution_clock::time_point start,end;

    for (uint64_t coord = 0; coord < params.coord_range; coord++) {
        start = std::chrono::high_resolution_clock::now();
        // Short circuit if we have previously PSI'd this coordinate.
        if (current_psi.count(coord)) continue;
        
        std::vector<cpp_share> rowshares(current_share_table.size());
        for(int i = 0; i < current_share_table.size(); i++){
            rowshares[i] = current_share_table[i][coord];
        }

        CombinationGen gen(0,current_share_table.size()-1,params.threshold);
        while (std::optional<std::vector<uint64_t>> combination = gen.next()){
            std::vector<cpp_share> subshare; // TODO
            for(uint64_t i : combination.value()){
                subshare.push_back(rowshares[i]);
            }
            // Expected decryption result is C-string "coord,round"
            std::memset(msgbuf,0,sss_MLEN);
            std::memset(outbuf,0,sss_MLEN);
            int msglen = std::snprintf((char*)msgbuf,sss_MLEN,"%d,%d",current_round,coord);
            CryptoPrimitives::sss_share_reconstruct(outbuf,sss_MLEN,subshare.data()->data(),subshare.size()*sizeof(subshare[0]),params.threshold);
            // If decryption is successful and the shared secret matches the expected result:
            if (CryptoPrimitives::secure_compare(msgbuf,outbuf,msglen)) {
                current_psi.insert(coord);
                break; // Short circuit for this coordinate.
            } else {
                // std::cout << "Row reconstruction failed. \nReported value:" << (char*)outbuf <<" Actual Value:"<<(char*) msgbuf<< std::endl;
            }
        }    
        end = std::chrono::high_resolution_clock::now();
        reconstruction_timings.push_back(std::chrono::duration_cast<std::chrono::duration<double>>(end-start));
    }
    // Finally, erase contents of share-matrix and start new round.
    current_share_table.clear();
    current_round++;

}

std::set<uint64_t> Reconstructor::get_psi()
{
    return current_psi;
}

void Reconstructor::print_stats()
{
    CryptoPrimitives::print_stats(row_dec_timings, "Row decryption");
    CryptoPrimitives::print_stats(reconstruction_timings, "Reconstruction");

    row_dec_timings.clear();
    reconstruction_timings.clear();
}
