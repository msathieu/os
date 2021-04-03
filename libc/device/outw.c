#include <__/syscall.h>

void outw(uint16_t port, uint16_t value) {
  _syscall(_SYSCALL_ACCESS_IOPORT, port, 1, 1, value, 0);
}
