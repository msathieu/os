#include <ipccalls.h>
#include <sys/types.h>

ssize_t write(int fd, const void* buffer, size_t size) {
  if (!size) {
    return 0;
  }
  int64_t return_value = send_ipc_call("vfsd", IPC_VFSD_WRITE, fd, 0, 0, (uintptr_t) buffer, size);
  if (return_value < 0) {
    return -1;
  } else {
    return return_value;
  }
}
