#include <cpu/ioports.h>
#include <cpuid.h>
#include <stdio.h>

int fputc(int c, __attribute__((unused)) void* file) {
  unsigned eax, ebx, ecx = 0, edx;
  __get_cpuid(1, &eax, &ebx, &ecx, &edx);
  if (!(ecx & 0x80000000)) {
    return 0;
  }
  outb(0x3f8, c);
  return c;
}
int puts(const char* str) {
  for (size_t i = 0; str[i]; i++) {
    putchar(str[i]);
  }
  putchar('\n');
  return 0;
}
int vfprintf(__attribute__((unused)) void* file, const char* restrict format, va_list args) {
  unsigned eax, ebx, ecx = 0, edx;
  __get_cpuid(1, &eax, &ebx, &ecx, &edx);
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
