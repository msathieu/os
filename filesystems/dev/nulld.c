#include <capability.h>
#include <ipc.h>

int main(void) {
  register_ipc(1);
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  while (1) {
    handle_ipc();
  }
}
