#include <ipc.h>
#include <stdlib.h>
#include <string.h>

ipc_handler ipc_handlers[256];

int64_t ipc_common(uint8_t syscall, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  if (!ipc_handlers[syscall]) {
    return -IPC_ERR_INVALID_SYSCALL;
  }
  if (syscall & IPC_CALL_MEMORY_SHARING) {
    void* address = malloc(arg4);
    memcpy(address, (void*) arg3, arg4);
    int64_t return_value = ipc_handlers[syscall](arg0, arg1, arg2, (uintptr_t) address, arg4);
    if (syscall & IPC_CALL_MEMORY_SHARING_RW_MASK) {
      memcpy((void*) arg3, address, arg4);
    }
    free(address);
    return return_value;
  } else {
    return ipc_handlers[syscall](arg0, arg1, arg2, arg3, arg4);
  }
}
