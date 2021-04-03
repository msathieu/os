#include <__/syscall.h>

void grant_ioport(uint16_t port) {
  _syscall(_SYSCALL_GRANT_IOPORT, port, 0, 0, 0, 0);
}
