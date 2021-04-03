#include <__/syscall.h>

void change_priority(int priority) {
  _syscall(_SYSCALL_CHANGE_PRIORITY, priority, 0, 0, 0, 0);
}
