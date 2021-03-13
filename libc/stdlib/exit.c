#include <__/syscall.h>

_Noreturn void exit(int status) {
  _syscall(_SYSCALL_EXIT, status, 0, 0, 0, 0);
  while (1)
    ;
}
