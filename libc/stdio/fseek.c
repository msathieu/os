#include <ipc.h>
#include <stdio.h>
#include <stdlib.h>

int fseek(FILE* file, long position, int mode) {
  if (mode != SEEK_SET) {
    exit(1);
  }
  return send_ipc_call("vfsd", 2, file->fd, position, 0, 0, 0);
}
