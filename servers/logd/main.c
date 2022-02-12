#include <__/syscall.h>
#include <capability.h>
#include <cpuid.h>
#include <ctype.h>
#include <ipccalls.h>
#include <stdlib.h>
#include <string.h>

static pid_t ttyd;
static char* buffer;
static bool log_kernel;

static int64_t registration_handler(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  if (arg0 || arg1 || arg2 || arg3 || arg4) {
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (!has_ipc_caller_capability(CAP_NAMESPACE_SERVERS, CAP_LOGD_TTY)) {
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  ttyd = get_ipc_caller_pid();
  return 0;
}
static int64_t log_handler(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size) {
  if (arg0 || arg1 || arg2) {
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (get_ipc_caller_uid()) {
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  char* msg = (char*) address;
  if (msg[size - 1]) {
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  for (size_t i = 0; i < size - 1; i++) {
    if (!isprint(msg[i])) {
      return -IPC_ERR_INVALID_ARGUMENTS;
    }
  }
  msg = malloc(size + 1);
  memcpy(msg, (void*) address, size);
  msg[size - 1] = '\n';
  msg[size] = 0;
  if (log_kernel) {
    for (size_t i = 0; i < size; i++) {
      _syscall(_SYSCALL_LOG, msg[i], 0, 0, 0, 0);
    }
  }
  if (ttyd) {
    send_pid_ipc_call(ttyd, IPC_VFSD_FS_WRITE, 0, 0, 0, (uintptr_t) msg, size);
  } else {
    if (!buffer) {
      buffer = strdup(msg);
    } else {
      buffer = realloc(buffer, strlen(buffer) + size + 1);
      strcat(buffer, msg);
    }
  }
  free(msg);
  return 0;
}
int main(void) {
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  register_ipc(1);
  ipc_set_started();
  unsigned eax, ebx, ecx, edx;
  __get_cpuid(1, &eax, &ebx, &ecx, &edx);
  if (ecx & 0x80000000) {
    log_kernel = 1;
  }
  ipc_handlers[IPC_LOGD_REGISTER] = registration_handler;
  ipc_handlers[IPC_LOGD_LOG] = log_handler;
  while (1) {
    handle_ipc();
    if (buffer && ttyd) {
      send_pid_ipc_call(ttyd, IPC_VFSD_FS_WRITE, 0, 0, 0, (uintptr_t) buffer, strlen(buffer));
      free(buffer);
      buffer = 0;
    }
  }
}
