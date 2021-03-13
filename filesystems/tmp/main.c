#include <capability.h>
#define TMP_MAGIC 0x5381835340924865

int main(void) {
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
}
