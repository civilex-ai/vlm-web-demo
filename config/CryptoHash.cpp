// CryptoHash.cpp - Implementation of the custom hash function.

#include "CryptoHash.h"
#include <cstring>
#include <stdexcept>

// Constants derived from the fractional parts of cube roots of the first 8 primes.
// This is a common technique to generate "nothing up my sleeve" numbers.
const std::array<uint32_t, 8> INITIAL_STATE = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
    0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
};

// Rotates x to the right by n bits.
inline uint32_t rotr(uint32_t x, uint32_t n) {
    return (x >> n) | (x << (32 - n));
}

// The core compression function logic. These are arbitrary choices.
inline uint32_t F0(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (~x & z);
}

inline uint32_t F1(uint32_t x, uint32_t y, uint32_t z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

CryptoHash::CryptoHash() {
    reset();
}

void CryptoHash::reset() {
    m_state = INITIAL_STATE;
    m_buffer_len = 0;
    m_bit_count = 0;
}

void CryptoHash::update(const uint8_t* data, size_t length) {
    if (!data) return;

    size_t buffer_space = 64 - m_buffer_len;
    if (length >= buffer_space) {
        // Fill the buffer and process it
        memcpy(m_buffer.data() + m_buffer_len, data, buffer_space);
        process_block(m_buffer.data());
        
        // Process remaining full blocks
        size_t i;
        for (i = buffer_space; i + 63 < length; i += 64) {
            process_block(data + i);
        }
        m_buffer_len = length - i;
        memcpy(m_buffer.data(), data + i, m_buffer_len);
    } else {
        memcpy(m_buffer.data() + m_buffer_len, data, length);
        m_buffer_len += length;
    }
    m_bit_count += length * 8;
}

void CryptoHash::update(const std::string& str) {
    update(reinterpret_cast<const uint8_t*>(str.c_str()), str.length());
}

Digest CryptoHash::finalize() {
    // Padding: append a single '1' bit (0x80)
    uint8_t padding[128];
    padding[0] = 0x80;
    
    // Append zeros until the length is 56 mod 64
    uint64_t current_len_mod_64 = (m_bit_count / 8) % 64;
    uint64_t pad_len = (current_len_mod_64 < 56) ? (56 - current_len_mod_64) : (120 - current_len_mod_64);
    
    memset(padding + 1, 0, pad_len - 1);

    // Append the original length in bits as a 64-bit big-endian integer
    for (int i = 0; i < 8; ++i) {
        padding[pad_len + i] = (m_bit_count >> (56 - i * 8)) & 0xFF;
    }

    update(padding, pad_len + 8);
    
    // Copy the final state to the digest
    Digest digest;
    for (int i = 0; i < 8; ++i) {
        digest[i*4 + 0] = (m_state[i] >> 24) & 0xFF;
        digest[i*4 + 1] = (m_state[i] >> 16) & 0xFF;
        digest[i*4 + 2] = (m_state[i] >> 8) & 0xFF;
        digest[i*4 + 3] = (m_state[i] >> 0) & 0xFF;
    }
    
    reset(); // Reset for next use
    return digest;
}

void CryptoHash::process_block(const uint8_t* block) {
    uint32_t w[64];
    for (int i = 0; i < 16; ++i) {
        w[i] = (block[i*4] << 24) | (block[i*4+1] << 16) | (block[i*4+2] << 8) | block[i*4+3];
    }
    
    // Extend the 16 32-bit words into 64 32-bit words
    for (int i = 16; i < 64; ++i) {
        uint32_t s0 = rotr(w[i-15], 7) ^ rotr(w[i-15], 18) ^ (w[i-15] >> 3);
        uint32_t s1 = rotr(w[i-2], 17) ^ rotr(w[i-2], 19) ^ (w[i-2] >> 10);
        w[i] = w[i-16] + s0 + w[i-7] + s1;
    }
    
    // Initialize working variables with current hash value
    uint32_t a = m_state[0], b = m_state[1], c = m_state[2], d = m_state[3];
    uint32_t e = m_state[4], f = m_state[5], g = m_state[6], h = m_state[7];
    
    // Compression function main loop
    for (int i = 0; i < 64; ++i) {
        uint32_t s1 = rotr(e, 6) ^ rotr(e, 11) ^ rotr(e, 25);
        uint32_t ch = (e & f) ^ (~e & g);
        uint32_t temp1 = h + s1 + ch + 0x428a2f98 + w[i]; // Using a single constant for simplicity
        uint32_t s0 = rotr(a, 2) ^ rotr(a, 13) ^ rotr(a, 22);
        uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
        uint32_t temp2 = s0 + maj;
        
        h = g;
        g = f;
        f = e;
        e = d + temp1;
        d = c;
        c = b;
        b = a;
        a = temp1 + temp2;
    }
    
    // Add the compressed chunk to the current hash value
    m_state[0] += a; m_state[1] += b; m_state[2] += c; m_state[3] += d;
    m_state[4] += e; m_state[5] += f; m_state[6] += g; m_state[7] += h;
}

Digest CryptoHash::compute(const std::string& data) {
    CryptoHash hasher;
    hasher.update(data);
    return hasher.finalize();
}
