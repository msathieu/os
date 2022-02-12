#include <__/syscall.h>
#include <capability.h>
#include <ipccalls.h>
#include <irq.h>
#include <lai/helpers/pm.h>
#include <lai/helpers/sci.h>
#include <pthread.h>
#include <syslog.h>

// TODO: Replace with mutex
static pthread_spinlock_t lock;

static int64_t power_state_handler(uint64_t type, __attribute__((unused)) uint64_t arg1, __attribute__((unused)) uint64_t arg2, __attribute__((unused)) uint64_t arg3, __attribute__((unused)) uint64_t arg4) {
  if (get_ipc_caller_uid()) {
    syslog(LOG_DEBUG, "No permission to change system power state");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  switch (type) {
  case 0:
    pthread_spin_lock(&lock);
    lai_enter_sleep(5);
    while (1) {
      asm volatile("pause");
    }
  case 1:
    pthread_spin_lock(&lock);
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
static int sci_handler(__attribute__((unused)) void* arg) {
  wait_irq();
  pthread_spin_lock(&lock);
  lai_enter_sleep(5);
  while (1) {
    asm volatile("pause");
  }
}
int main(void) {
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  register_ipc(0);
  ipc_set_started();
  int revision = _syscall(_SYSCALL_GET_ACPI_REVISION, 0, 0, 0, 0, 0);
  lai_set_acpi_revision(revision);
  lai_create_namespace();
  lai_enable_acpi(1);
  lai_set_sci_event(ACPI_POWER_BUTTON);
  pthread_spin_init(&lock, 0);
  thrd_create(0, sci_handler, 0);
  register_ipc_call(IPC_ACPID_POWER_STATE, power_state_handler, 1);
  while (1) {
    handle_ipc();
  }
}
