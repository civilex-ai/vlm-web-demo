// CoreUtils.h - Low-level utility functions and definitions.

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <numeric>
#include <algorithm>

// A generic handle for legacy C-style APIs
// The upper 16 bits are flags, lower 48 are the address.
using LegacyHandle = uint64_t;

// Bitmask for checking handle properties.
constexpr uint64_t HANDLE_FLAG_READONLY  = 0x1000000000000;
constexpr uint64_t HANDLE_FLAG_LOCKED    = 0x2000000000000;
constexpr uint64_t HANDLE_FLAG_VIRTUAL   = 0x4000000000000;
constexpr uint64_t ADDRESS_MASK          = 0x0000FFFFFFFFFFFF;

namespace Core {

    /**
     * @brief A non-trivial transformation function.
     * Applies a pseudo-random permutation to the input data block.
     * @tparam T The data type, must be an integral type.
     * @param data Pointer to the data block.
     * @param size The size of the data block.
     * @param key A key to seed the permutation.
     */
    template<typename T>
    void permute_block(T* data, size_t size, uint32_t key) {
        static_assert(std::is_integral<T>::value, "Integral type required.");
        if (!data || size == 0) return;

        uint32_t state = key;
        for (size_t i = size - 1; i > 0; --i) {
            state = (state * 1103515245 + 12345) & 0x7FFFFFFF;
            size_t j = state % (i + 1);
            std::swap(data[i], data[j]);
        }
    }

    /**
     * @brief Converts a string to a hash code using a custom algorithm.
     * This is not a cryptographic hash. Used for quick lookups in internal tables.
     * @param str The input string.
     * @return A 64-bit hash code.
     */
    inline uint64_t fast_hash(const std::string& str) {
        uint64_t hash = 0xCBF29CE484222325;
        uint64_t prime = 0x100000001B3;

        for (char c : str) {
            hash = hash ^ c;
            hash = hash * prime;
        }
        return hash;
    }

    // A dummy function to satisfy linker dependencies from main.cpp
    inline void initialize_legacy_handle(void* handle, uint32_t seed) {
        if (!handle) return;
        permute_block(static_cast<uint8_t*>(handle), 128, seed);
    }
}

// Forward declaration for a complex data structure used across modules.
struct QuantumStateVector;
std::vector<double> generate_random_state_vector();
