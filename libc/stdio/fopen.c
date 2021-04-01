#include <ipccalls.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE* fopen(const char* restrict path, const char* restrict arg_mode) {
  size_t mode;
  if (!strcmp(arg_mode, "r")) {
    mode = 0;
  } else if (!strcmp(arg_mode, "w")) {
    mode = 1;
  } else {
    exit(1);
  }
  int64_t fd = send_ipc_call("vfsd", IPC_VFSD_OPEN, mode, 0, 0, (uintptr_t) path, strlen(path) + 1);
  if (fd >= 0) {
    FILE* file = calloc(1, sizeof(FILE));
    file->fd = fd;
    return file;
  } else {
    return 0;
  }
}
