#include <pthread.h>

int pthread_spin_unlock(pthread_spinlock_t* spinlock) {
  __atomic_thread_fence(__ATOMIC_SEQ_CST);
  *spinlock = 0;
  return 0;
}
