#ifndef PRVAK_MATH_H
#define PRVAK_MATH_H

#include <stdbool.h>
#include <stdint.h>

#define CLZ64(x) (__builtin_clzll(x))
#define CTZ64(x) (__builtin_ctzll(x))
#define floor_log2(x) (sizeof(uint64_t) * 8 - 1 - CLZ64(x))

// TODO: (x & (x - 1)) == 0
#define is_pow2(x) ((int) floor_log2(x) == CTZ64(x))

#define hyperfloor(x) (1ULL << floor_log2(x))
#define ceil_log2(x) (is_pow2(x) ? floor_log2(x) : (floor_log2(x) + 1))

uint8_t exact_log2(uint64_t x);
uint64_t ceil_div(uint64_t a, uint64_t b);

#endif
