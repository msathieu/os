#include <ipccalls.h>
#include <stdio.h>

long ftell(FILE* file) {
  int64_t return_value = send_ipc_call("vfsd", IPC_VFSD_SEEK, file->fd, SEEK_CUR, 0, 0, 0);
  if (return_value < 0) {
    return -1;
  } else {
    return return_value;
  }
}
