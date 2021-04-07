#include <ipccalls.h>
#include <string.h>
#include <sys/types.h>

ssize_t read(int fd, void* buffer, size_t size) {
  memset(buffer, 0, size);
  return send_ipc_call("vfsd", IPC_VFSD_READ, fd, 0, 0, (uintptr_t) buffer, size);
}
