#include <__/syscall.h>

void clear_irqs(void) {
  _syscall(_SYSCALL_CLEAR_IRQS, 0, 0, 0, 0, 0);
}
int wait_irq(void) {
  return _syscall(_SYSCALL_WAIT_IRQ, 0, 0, 0, 0, 0);
}
