#include <__/syscall.h>

uint32_t inl(uint16_t port) {
  return _syscall(_SYSCALL_ACCESS_IOPORT, port, 0, 2, 0, 0);
}
