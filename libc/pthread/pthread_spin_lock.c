#include <pthread.h>

int pthread_spin_lock(pthread_spinlock_t* spinlock) {
  while (pthread_spin_trylock(spinlock)) {
    asm volatile("pause");
  }
  return 0;
}
