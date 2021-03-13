#include <ipc.h>
#include <stdio.h>

size_t fread(void* restrict buffer, size_t size, size_t num, FILE* restrict file) {
  send_ipc_call("vfsd", IPC_CALL_MEMORY_SHARING_RW, file->fd, 0, 0, (uintptr_t) buffer, size * num);
  return num;
}
