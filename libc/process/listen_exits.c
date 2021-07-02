#include <__/syscall.h>

void listen_exits(void) {
  _syscall(_SYSCALL_LISTEN_EXITS, 0, 0, 0, 0, 0);
}
