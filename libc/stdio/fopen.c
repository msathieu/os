#include <ipccalls.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE* fopen(const char* restrict path, __attribute__((unused)) const char* restrict mode) {
  int64_t fd = send_ipc_call("vfsd", IPC_VFSD_OPEN, 0, 0, 0, (uintptr_t) path, strlen(path) + 1);
  if (fd >= 0) {
    FILE* file = calloc(1, sizeof(FILE));
    file->fd = fd;
    return file;
  } else {
    return 0;
  }
}
