#include <__/rpmalloc.h>
#include <__/syscall.h>

//TODO: Free thread
_Noreturn void thrd_exit(int status) {
  rpmalloc_thread_finalize(1);
  _syscall(_SYSCALL_EXIT, status, 0, 0, 0, 0);
  while (1)
    ;
}
