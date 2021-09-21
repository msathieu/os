#include <threads.h>

thrd_t thrd_current(void) {
  thrd_t thread;
  asm volatile("mov %%fs:0, %0"
               : "=r"(thread));
  return thread;
}
