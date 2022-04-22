#include <stdio.h>
#include <string.h>

int vfprintf(FILE* restrict file, const char* restrict format, va_list args) {
  char buf[1024];
  int return_value = vsnprintf(buf, 1024, format, args);
  fwrite(buf, 1, strlen(buf), file);
  return return_value;
}
