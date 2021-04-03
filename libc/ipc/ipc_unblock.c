#include <__/syscall.h>
#include <unistd.h>

void ipc_unblock(pid_t pid) {
  _syscall(_SYSCALL_UNBLOCK_IPC_CALL, pid, 0, 0, 0, 0);
}
