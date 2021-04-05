#include <capability.h>
#include <ipc.h>
#include <lai/helpers/sci.h>

int main(void) {
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  register_ipc(0);
  lai_create_namespace();
  lai_enable_acpi(1);
  while (1) {
    handle_ipc();
  }
}
