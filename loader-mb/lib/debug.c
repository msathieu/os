#include <cpu/ioports.h>
#include <cpuid.h>
#include <panic.h>
#include <stdio.h>
#include <struct.h>

int fputc(int c, __attribute__((unused)) void* file) {
  if (loader_struct.debug_port) {
    outb(loader_struct.debug_port, c);
  }
  return c;
}
int vfprintf(__attribute__((unused)) void* file, const char* restrict format, va_list args) {
  char buf[512];
  int return_value = vsnprintf(buf, 512, format, args);
  for (size_t i = 0; buf[i]; i++) {
    putchar(buf[i]);
  }
  return return_value;
}
