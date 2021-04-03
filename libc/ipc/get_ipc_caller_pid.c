#include <__/syscall.h>
#include <unistd.h>

uid_t get_ipc_caller_uid(void) {
  return _syscall(_SYSCALL_GET_UID, 1, 0, 0, 0, 0);
}
