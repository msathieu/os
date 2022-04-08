#include <stdio.h>

int vprintf(const char* restrict format, va_list args) {
  return vfprintf(stdout, format, args);
}
