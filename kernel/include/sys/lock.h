#pragma once
#include <stdbool.h>

extern bool smp_lock;

static inline void acquire_lock(void) {
  bool expected = 0;
  while (!__atomic_compare_exchange_n(&smp_lock, &expected, 1, 0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED)) {
    expected = 0;
    asm volatile("pause");
  }
  __atomic_thread_fence(__ATOMIC_SEQ_CST);
}
static inline void release_lock(void) {
  __atomic_thread_fence(__ATOMIC_SEQ_CST);
  smp_lock = 0;
}
