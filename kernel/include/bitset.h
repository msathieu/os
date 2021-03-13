#pragma once
#include <stddef.h>
#include <stdint.h>

static inline void bitset_set(uint64_t* bitset, size_t i) {
  bitset[i / 64] |= 1 << (i % 64);
}
static inline void bitset_clear(uint64_t* bitset, size_t i) {
  bitset[i / 64] &= ~(1 << (i % 64));
}
static inline size_t bitset_test(uint64_t* bitset, size_t i) {
  return bitset[i / 64] & 1 << (i % 64);
}
