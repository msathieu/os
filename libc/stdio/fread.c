#include <ipccalls.h>
#include <stdio.h>
#include <string.h>

size_t fread(void* restrict buffer, size_t size, size_t num, FILE* restrict file) {
  memset(buffer, 0, size * num);
  return send_ipc_call("vfsd", IPC_VFSD_READ, file->fd, 0, 0, (uintptr_t) buffer, size * num) / size;
}
