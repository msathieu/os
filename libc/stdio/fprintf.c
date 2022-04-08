#include <stdio.h>

int fprintf(FILE* restrict file, const char* restrict format, ...) {
  va_list args;
  va_start(args, format);
  int return_value = vfprintf(file, format, args);
  va_end(args);
  return return_value;
}
