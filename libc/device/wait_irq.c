#include <__/syscall.h>

int wait_irq(void) {
  return _syscall(_SYSCALL_WAIT_IRQ, 0, 0, 0, 0, 0);
}
