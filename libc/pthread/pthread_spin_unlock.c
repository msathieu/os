#include <pthread.h>

int pthread_spin_unlock(pthread_spinlock_t* spinlock) {
  __sync_synchronize();
  *spinlock = 0;
  return 0;
}
