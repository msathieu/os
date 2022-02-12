#include <__/ipc.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

struct ipc_call _ipc_calls[256];

int64_t ipc_common(uint8_t syscall, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  if (!_ipc_calls[syscall].handler) {
    return -IPC_ERR_INVALID_SYSCALL;
  }
  if ((_ipc_calls[syscall].nargs < 5 && arg4 && !(syscall & IPC_CALL_MEMORY_SHARING)) ||
      (_ipc_calls[syscall].nargs < 4 && arg3 && !(syscall & IPC_CALL_MEMORY_SHARING)) ||
      (_ipc_calls[syscall].nargs < 3 && arg2) ||
      (_ipc_calls[syscall].nargs < 2 && arg1) ||
      (_ipc_calls[syscall].nargs < 1 && arg0)) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (syscall & IPC_CALL_MEMORY_SHARING) {
    void* address = malloc(arg4);
    memcpy(address, (void*) arg3, arg4);
    int64_t return_value = _ipc_calls[syscall].handler(arg0, arg1, arg2, (uintptr_t) address, arg4);
    if (syscall & IPC_CALL_MEMORY_SHARING_RW_MASK) {
      memcpy((void*) arg3, address, arg4);
    }
    free(address);
    return return_value;
  } else {
    return _ipc_calls[syscall].handler(arg0, arg1, arg2, arg3, arg4);
  }
}
