#include <__/syscall.h>

void unregister_ipc(void) {
  _syscall(_SYSCALL_REGISTER_IPC, 0, 0, 0, 0, 0);
}
