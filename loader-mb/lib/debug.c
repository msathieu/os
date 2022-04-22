#include <cpu/ioports.h>
#include <cpuid.h>
#include <panic.h>
#include <stdio.h>
#include <struct.h>

int putchar(int c) {
  if (loader_struct.debug_port) {
    outb(loader_struct.debug_port, c);
  }
  return c;
}
int vprintf(const char* restrict format, va_list args) {
  char buf[1024];
  int return_value = vsnprintf(buf, 1024, format, args);
  for (size_t i = 0; buf[i]; i++) {
    putchar(buf[i]);
  }
  return return_value;
}
