#include <__/syscall.h>

void grant_capability(int namespace, int capability) {
  _syscall(_SYSCALL_GRANT_CAPABILITIES, namespace, 1 << capability, 0, 0, 0);
}
