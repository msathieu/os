#include <ipccalls.h>
#include <string.h>
#include <sys/types.h>

ssize_t read(int fd, void* buffer, size_t size) {
  if (!size) {
    return 0;
  }
  memset(buffer, 0, size);
  int64_t return_value = send_ipc_call("vfsd", IPC_VFSD_READ, fd, 0, 0, (uintptr_t) buffer, size);
  if (return_value < 0) {
    return -1;
  } else {
    return return_value;
  }
}
