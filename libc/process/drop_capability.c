#include <__/syscall.h>

void drop_capability(int namespace, int capability) {
  _syscall(_SYSCALL_DROP_CAPABILITIES, namespace, 1 << capability, 0, 0, 0);
}
