#include <capability.h>
#include <ipccalls.h>
#define TMP_MAGIC 0x5381835340924865

int main(void) {
  register_ipc(1);
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  while (1) {
    handle_ipc();
  }
}
