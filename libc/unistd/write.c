#include <ipccalls.h>
#include <sys/types.h>

ssize_t write(int fd, const void* buffer, size_t size) {
  return send_ipc_call("vfsd", IPC_VFSD_WRITE, fd, 0, 0, (uintptr_t) buffer, size);
}
