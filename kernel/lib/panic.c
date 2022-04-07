#include <stdio.h>
#include <sys/lapic.h>

_Noreturn void _panic(const char* msg, const char* file, int line, const char* fn) {
  printf("PANIC: %s (%s:%d in %s)\n", msg, file, line, fn);
  smp_broadcast_nmi();
  while (true) {
    asm volatile("hlt");
  }
}
