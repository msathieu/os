#include <__/syscall.h>

void outl(uint16_t port, uint32_t value) {
  _syscall(_SYSCALL_ACCESS_IOPORT, port, 1, 2, value, 0);
}
