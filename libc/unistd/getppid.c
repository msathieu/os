#include <__/syscall.h>
#include <sys/types.h>

pid_t getppid(void) {
  return _syscall(_SYSCALL_GET_PARENT_PID, 0, 0, 0, 0, 0);
}
