#include <errno.h>
#include <pthread.h>

int pthread_spin_trylock(pthread_spinlock_t* spinlock) {
  bool expected = 0;
  if (__atomic_compare_exchange_n(spinlock, &expected, 1, 0, __ATOMIC_SEQ_CST, __ATOMIC_RELAXED)) {
    __atomic_thread_fence(__ATOMIC_SEQ_CST);
    return 0;
  } else {
    return EBUSY;
  }
}
