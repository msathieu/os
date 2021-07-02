#include <__/syscall.h>
#include <sys/types.h>

pid_t get_exited_pid(void) {
  return _syscall(_SYSCALL_GET_EXITED_PID, 0, 0, 0, 0, 0);
}
