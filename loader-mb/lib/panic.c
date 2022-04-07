#include <stdbool.h>
#include <stdio.h>

_Noreturn void _panic(const char* msg, const char* file, int line, const char* fn) {
  printf("PANIC: %s (%s:%d in %s)\n", msg, file, line, fn);
  while (true) {
    asm volatile("hlt");
  }
}
