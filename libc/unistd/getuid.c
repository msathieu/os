#include <__/syscall.h>
#include <sys/types.h>

uid_t getuid(void) {
  return _syscall(_SYSCALL_GET_UID, 0, 0, 0, 0, 0);
}
