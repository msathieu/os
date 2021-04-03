#include <__/syscall.h>

uint16_t inw(uint16_t port) {
  return _syscall(_SYSCALL_ACCESS_IOPORT, port, 0, 1, 0, 0);
}
