#pragma once
#include <stdbool.h>

extern bool smp_lock;

static inline void acquire_lock(void) {
  bool expected = false;
  while (!__atomic_compare_exchange_n(&smp_lock, &expected, true, false, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED)) {
    expected = false;
    asm volatile("pause");
  }
  __atomic_thread_fence(__ATOMIC_SEQ_CST);
}
static inline void release_lock(void) {
  __atomic_thread_fence(__ATOMIC_SEQ_CST);
  smp_lock = false;
}
