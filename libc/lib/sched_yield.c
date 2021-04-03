#include <__/syscall.h>

int sched_yield(void) {
  _syscall(_SYSCALL_YIELD, 0, 0, 0, 0, 0);
  return 0;
}
