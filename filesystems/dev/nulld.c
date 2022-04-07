#include <capability.h>
#include <ipccalls.h>
#include <syslog.h>

static int64_t read_handler(__attribute__((unused)) uint64_t offset, __attribute__((unused)) uint64_t arg1, __attribute__((unused)) uint64_t arg2, __attribute__((unused)) uint64_t address, __attribute__((unused)) uint64_t size) {
  return 0;
}
static int64_t write_handler(__attribute__((unused)) uint64_t offset, __attribute__((unused)) uint64_t arg1, __attribute__((unused)) uint64_t arg2, __attribute__((unused)) uint64_t address, uint64_t size) {
  return size;
}
int main(void) {
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  register_ipc(true);
  register_ipc_call(IPC_VFSD_FS_WRITE, write_handler, 1);
  register_ipc_call(IPC_VFSD_FS_READ, read_handler, 1);
  while (true) {
    handle_ipc();
  }
}
