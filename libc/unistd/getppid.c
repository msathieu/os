#include <__/syscall.h>
#include <sys/types.h>

pid_t getppid(void) {
  return _syscall(_SYSCALL_GET_PID, 2, 0, 0, 0, 0);
}
