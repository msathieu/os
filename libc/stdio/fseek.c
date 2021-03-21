#include <ipccalls.h>
#include <stdio.h>
#include <stdlib.h>

int fseek(FILE* file, long position, int mode) {
  if (mode != SEEK_SET) {
    exit(1);
  }
  return send_ipc_call("vfsd", IPC_VFSD_SEEK, file->fd, position, 0, 0, 0);
}
