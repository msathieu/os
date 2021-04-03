#include <__/syscall.h>
#include <stdbool.h>

void register_ipc(bool accepts_shared_memory) {
  _syscall(_SYSCALL_REGISTER_IPC, 1, accepts_shared_memory, 0, 0, 0);
}
