#include <__/syscall.h>
#include <sys/types.h>

pid_t wait(__attribute__((unused)) int* status) {
  return _syscall(_SYSCALL_WAIT, 0, 0, 0, 0, 0);
}
