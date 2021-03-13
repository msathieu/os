#include <__/syscall.h>
#include <time.h>

time_t time(time_t* location) {
  size_t time = _syscall(_SYSCALL_GET_TIME, 0, 0, 0, 0, 0) / 1000;
  if (location) {
    *location = time;
  }
  return time;
}
