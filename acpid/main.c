#include <__/syscall.h>
#include <capability.h>
#include <ipc.h>
#include <lai/helpers/sci.h>

int main(void) {
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  register_ipc(0);
  int revision = _syscall(_SYSCALL_GET_ACPI_REVISION, 0, 0, 0, 0, 0);
  lai_set_acpi_revision(revision);
  lai_create_namespace();
  lai_enable_acpi(1);
  while (1) {
    handle_ipc();
  }
}
