#include <__/syscall.h>

void register_irq(int irq) {
  _syscall(_SYSCALL_REGISTER_IRQ, 0, irq, 0, 0, 0);
}
