#include <ipccalls.h>
#include <stdio.h>

int fgetc(FILE* file) {
  unsigned char c;
  size_t return_value = fread(&c, 1, 1, file);
  if (return_value) {
    return c;
  } else {
    return EOF;
  }
}
