#include <__/syscall.h>
#include <stdbool.h>

void register_ipc(void) {
  _syscall(_SYSCALL_REGISTER_IPC, 0, 0, 0, 0, 0);
}
