#include <cpu/ioports.h>
#include <cpuid.h>
#include <panic.h>
#include <stdio.h>

int putc(int c, __attribute__((unused)) void* file) {
  unsigned eax, ebx, ecx, edx;
  if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
    panic("Error obtaining CPUID information");
  }
  if (!(ecx & 0x80000000)) {
    return 0;
  }
  outb(0x3f8, c);
  return c;
}
int vfprintf(__attribute__((unused)) void* file, const char* restrict format, va_list args) {
  unsigned eax, ebx, ecx, edx;
  if (!__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
    panic("Error obtaining CPUID information");
  }
  if (!(ecx & 0x80000000)) {
    return 0;
  }
  char buf[512];
  int return_value = vsnprintf(buf, 512, format, args);
  for (size_t i = 0; buf[i]; i++) {
    outb(0x3f8, buf[i]);
  }
  return return_value;
}
