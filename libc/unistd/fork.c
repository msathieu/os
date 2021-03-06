#include <__/syscall.h>
#include <ipccalls.h>
#include <string.h>

extern char** environ;

pid_t fork(void) {
  _syscall(_SYSCALL_FORK, 0, 0, 0, 0, 0);
  for (size_t i = 0; environ[i]; i++) {
    send_ipc_call("argd", IPC_ARGD_ADD, 1, 0, 0, (uintptr_t) environ[i], strlen(environ[i]) + 1);
  }
  send_ipc_call("vfsd", IPC_VFSD_CLONE_FDS, 1, 0, 0, 0, 0);
  return _syscall(_SYSCALL_START_FORK, 0, 0, 0, 0, 0);
}
