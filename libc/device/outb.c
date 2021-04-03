#include <__/syscall.h>

void outb(uint16_t port, uint8_t value) {
  _syscall(_SYSCALL_ACCESS_IOPORT, port, 1, 0, value, 0);
}
