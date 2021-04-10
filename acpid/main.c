#include <__/syscall.h>
#include <capability.h>
#include <ipccalls.h>
#include <lai/helpers/pm.h>
#include <lai/helpers/sci.h>
#include <syslog.h>

static int64_t power_state_handler(uint64_t type, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  if (arg1 || arg2 || arg3 || arg4) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (get_ipc_caller_uid()) {
    syslog(LOG_DEBUG, "No permission to change system power state");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  switch (type) {
  case 0:
    lai_enter_sleep(5);
    while (1) {
      asm volatile("pause");
    }
  case 1:;
    lai_api_error_t error = lai_acpi_reset();
    if (error) {
      _syscall(_SYSCALL_RESET, 0, 0, 0, 0, 0);
    }
    while (1) {
      asm volatile("pause");
    }
  default:
    syslog(LOG_DEBUG, "Argument out of range");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
}
int main(void) {
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  register_ipc(0);
  int revision = _syscall(_SYSCALL_GET_ACPI_REVISION, 0, 0, 0, 0, 0);
  lai_set_acpi_revision(revision);
  lai_create_namespace();
  lai_enable_acpi(1);
  ipc_handlers[IPC_ACPID_POWER_STATE] = power_state_handler;
  while (1) {
    handle_ipc();
  }
}
