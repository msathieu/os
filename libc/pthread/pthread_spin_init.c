#include <pthread.h>

int pthread_spin_init(pthread_spinlock_t* spinlock, __attribute__((unused)) int shared) {
  *spinlock = 0;
  return 0;
}
