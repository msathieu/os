#include <__/syscall.h>
#include <stdbool.h>

bool is_caller_child(void) {
  return _syscall(_SYSCALL_IS_CALLER_CHILD, 0, 0, 0, 0, 0);
}
