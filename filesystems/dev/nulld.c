#include <capability.h>
#include <ipccalls.h>
#include <syslog.h>

static int64_t read_handler(__attribute__((unused)) uint64_t offset, uint64_t arg1, uint64_t arg2, __attribute__((unused)) uint64_t address, __attribute__((unused)) uint64_t size) {
  if (arg1 || arg2) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  return 0;
}
static int64_t write_handler(__attribute__((unused)) uint64_t offset, uint64_t arg1, uint64_t arg2, __attribute__((unused)) uint64_t address, uint64_t size) {
  if (arg1 || arg2) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  return size;
}
int main(void) {
  register_ipc(1);
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  ipc_handlers[IPC_VFSD_FS_WRITE] = write_handler;
  ipc_handlers[IPC_VFSD_FS_READ] = read_handler;
  while (1) {
    handle_ipc();
  }
}
