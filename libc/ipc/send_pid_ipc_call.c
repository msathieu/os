#include <__/syscall.h>
#include <ipc.h>
#include <stdlib.h>
#include <string.h>

int64_t send_pid_ipc_call(pid_t pid, uint8_t syscall, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  uintptr_t buffer = arg3;
  if (syscall & IPC_CALL_MEMORY_SHARING) {
    buffer = (uintptr_t) aligned_alloc(0x1000, (arg4 + 0xfff) / 0x1000 * 0x1000);
    memset((void*) buffer + arg4, 0, (arg4 + 0xfff) / 0x1000 * 0x1000 - arg4);
    memcpy((void*) buffer, (void*) arg3, arg4);
  }
  int64_t return_value = _syscall(pid << 8 | syscall, arg0, arg1, arg2, buffer, arg4);
  if (syscall & IPC_CALL_MEMORY_SHARING) {
    if (syscall & IPC_CALL_MEMORY_SHARING_RW_MASK) {
      memcpy((void*) arg3, (void*) buffer, arg4);
    }
    free((void*) buffer);
  }
  return return_value;
}
