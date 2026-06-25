#ifndef BITOPS_H
#define BITOPS_H

#include <cstdint>

// Portable bit manipulation utilities that work on GCC, Clang, and MSVC

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace BitOps {

// Count trailing zeros (returns position of least significant 1-bit)
inline int ctzll(uint64_t x) {
    if (x == 0) return 64;
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_ctzll(x);
#elif defined(_MSC_VER)
    unsigned long index;
    _BitScanForward64(&index, x);
    return static_cast<int>(index);
#else
    int n = 0;
    while ((x & 1) == 0) { x >>= 1; ++n; }
    return n;
#endif
}

// Population count (number of 1-bits)
inline int popcountll(uint64_t x) {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_popcountll(x);
#elif defined(_MSC_VER)
    return static_cast<int>(__popcnt64(x));
#else
    x = x - ((x >> 1) & 0x5555555555555555ULL);
    x = (x & 0x3333333333333333ULL) + ((x >> 2) & 0x3333333333333333ULL);
    x = (x + (x >> 4)) & 0x0F0F0F0F0F0F0F0FULL;
    return static_cast<int>((x * 0x0101010101010101ULL) >> 56);
#endif
}

// Isolate the least significant 1-bit
inline uint64_t lsb(uint64_t x) {
    return x & -x;
}

// Extract the index of the least significant 1-bit and clear it
inline int pop_lsb(uint64_t& bb) {
    uint64_t lsb_val = lsb(bb);
    bb ^= lsb_val;
    return ctzll(lsb_val);
}

// Get the least significant 1-bit's index without clearing
inline int get_lsb_index(uint64_t bb) {
    return ctzll(bb);
}

} // namespace BitOps

#endif // BITOPS_H
