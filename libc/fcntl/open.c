#include <ipccalls.h>
#include <string.h>

int open(const char* path, int flags, ...) {
  int64_t fd = send_ipc_call("vfsd", IPC_VFSD_OPEN, flags, 0, 0, (uintptr_t) path, strlen(path) + 1);
  if (fd < 0) {
    return -1;
  } else {
    return fd;
  }
}
