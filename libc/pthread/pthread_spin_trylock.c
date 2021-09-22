#include <errno.h>
#include <pthread.h>

int pthread_spin_trylock(pthread_spinlock_t* spinlock) {
  if (__sync_bool_compare_and_swap(spinlock, 0, 1)) {
    __sync_synchronize();
    return 0;
  } else {
    return EBUSY;
  }
}
