#include <__/syscall.h>
#include <stdbool.h>
#include <stddef.h>

bool has_ipc_caller_capability(int namespace, int capability) {
  size_t capabilities = _syscall(_SYSCALL_GET_IPC_CALLER_CAPABILITIES, namespace, 0, 0, 0, 0);
  return capabilities & (1 << capability);
}
