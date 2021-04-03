#include <__/syscall.h>
#include <unistd.h>

pid_t get_ipc_caller_pid(void) {
  return _syscall(_SYSCALL_GET_PID, 1, 0, 0, 0, 0);
}
