#include <ipc.h>
#include <stdio.h>
#include <stdlib.h>

int fclose(FILE* file) {
  send_ipc_call("vfsd", 1, file->fd, 0, 0, 0, 0);
  free(file);
  return 0;
}
