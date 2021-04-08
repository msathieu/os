#include <ipccalls.h>
#include <stdio.h>

int fseek(FILE* file, long position, int mode) {
  int64_t return_value = send_ipc_call("vfsd", IPC_VFSD_SEEK, file->fd, mode, position, 0, 0);
  if (return_value < 0) {
    return -1;
  } else {
    return 0;
  }
}
