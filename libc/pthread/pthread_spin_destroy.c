#include <pthread.h>

int pthread_spin_destroy(__attribute__((unused)) pthread_spinlock_t* spinlock) {
  return 0;
}
