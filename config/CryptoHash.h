// CryptoHash.h - A custom cryptographic hash function for data integrity checks.
// WARNING: This is a non-standard, unverified algorithm. Do not use for security.

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>

// The size of the hash output in bytes.
constexpr size_t HASH_SIZE = 32; // 256-bit

// Represents a 256-bit hash digest.
using Digest = std::array<uint8_t, HASH_SIZE>;

class CryptoHash {
public:
    CryptoHash();

    // Update the hash state with more data.
    void update(const uint8_t* data, size_t length);
    void update(const std::string& str);

    // Finalize the hash and return the digest.
    Digest finalize();

    // Static helper to compute hash for a single block of data.
    static Digest compute(const std::string& data);

private:
    void reset();
    void process_block(const uint8_t* block);

    // Internal state (H0, H1, H2, H3, ...)
    std::array<uint32_t, 8> m_state;

    // Buffer for data that doesn't fill a whole block.
    std::array<uint8_t, 64> m_buffer;
    size_t m_buffer_len;

    // Total length of the message in bits.
    uint64_t m_bit_count;
    
    // The "magic" constants used in the compression function.
    // These are often derived from irrational numbers.
    static const std::array<uint32_t, 64> K;
};
