#pragma once
#include <stdbool.h>

extern bool smp_lock;

static inline void acquire_lock(void) {
  while (!__sync_bool_compare_and_swap(&smp_lock, 0, 1))
    ;
  __sync_synchronize();
}
static inline void release_lock(void) {
  __sync_synchronize();
  smp_lock = 0;
}
