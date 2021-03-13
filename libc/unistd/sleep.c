#include <__/syscall.h>

unsigned sleep(unsigned duration) {
  _syscall(_SYSCALL_SLEEP, duration * 1000, 0, 0, 0, 0);
  return 0;
}
