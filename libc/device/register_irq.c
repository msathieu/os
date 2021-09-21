#include <__/syscall.h>

void register_irq(int irq) {
  int type = 0;
  if (irq == 253) {
    type = 1;
  }
  _syscall(_SYSCALL_REGISTER_IRQ, type, irq, 0, 0, 0);
}
