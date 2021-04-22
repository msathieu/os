#include <ipccalls.h>

int close(int fd) {
  int64_t return_value = send_ipc_call("vfsd", IPC_VFSD_CLOSE, fd, 0, 0, 0, 0);
  if (return_value) {
    return -1;
  } else {
    return 0;
  }
}
