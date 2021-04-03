#include <__/syscall.h>

uint8_t inb(uint16_t port) {
  return _syscall(_SYSCALL_ACCESS_IOPORT, port, 0, 0, 0, 0);
}
