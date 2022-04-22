#include <cpu/ioports.h>
#include <stdio.h>
#include <struct.h>

int putchar(int c) {
  if (loader_struct.debug_port) {
    outb(loader_struct.debug_port, c);
  }
  return c;
}
int puts(const char* str) {
  for (size_t i = 0; str[i]; i++) {
    putchar(str[i]);
  }
  putchar('\n');
  return 0;
}
int vprintf(const char* restrict format, va_list args) {
  char buf[1024];
  int return_value = vsnprintf(buf, 1024, format, args);
  for (size_t i = 0; buf[i]; i++) {
    putchar(buf[i]);
  }
  return return_value;
}
