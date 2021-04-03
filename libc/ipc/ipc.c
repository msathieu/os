#include <ipc.h>

ipc_handler ipc_handlers[256];

int64_t ipc_common(uint8_t syscall, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  if (!ipc_handlers[syscall]) {
    return -IPC_ERR_INVALID_SYSCALL;
  }
  return ipc_handlers[syscall](arg0, arg1, arg2, arg3, arg4);
}
