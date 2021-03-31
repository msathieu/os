#include <ipccalls.h>
#include <stdio.h>

size_t fwrite(const void* restrict buffer, size_t size, size_t num, FILE* restrict file) {
  return send_ipc_call("vfsd", IPC_VFSD_WRITE, file->fd, 0, 0, (uintptr_t) buffer, size * num) / size;
}
