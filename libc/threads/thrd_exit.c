#include <pthread.h>
#include <stdint.h>

_Noreturn void thrd_exit(int status) {
  pthread_exit((void*) (uintptr_t) status);
}
