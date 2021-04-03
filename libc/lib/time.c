#include <__/syscall.h>
#include <time.h>

time_t time(time_t* ptr) {
  size_t time = _syscall(_SYSCALL_GET_TIME, 0, 0, 0, 0, 0) / 1000;
  if (ptr) {
    *ptr = time;
  }
  return time;
}
