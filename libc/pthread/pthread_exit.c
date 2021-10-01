#include <__/rpmalloc.h>
#include <__/syscall.h>

// TODO: Free thread
_Noreturn void pthread_exit(void* value) {
  rpmalloc_thread_finalize(1);
  _syscall(_SYSCALL_EXIT, (uintptr_t) value, 0, 0, 0, 0);
  while (1)
    ;
}
