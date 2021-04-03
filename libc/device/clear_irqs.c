#include <__/syscall.h>

void clear_irqs(void) {
  _syscall(_SYSCALL_CLEAR_IRQS, 0, 0, 0, 0, 0);
}
