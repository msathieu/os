#include <__/syscall.h>
#include <sys/types.h>

pid_t getpid(void) {
  return _syscall(_SYSCALL_GET_PID, 0, 0, 0, 0, 0);
}
