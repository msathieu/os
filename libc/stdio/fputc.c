#include <stdio.h>

int fputc(int c, FILE* file) {
  unsigned char arg = c;
  size_t return_value = fwrite(&arg, 1, 1, file);
  if (return_value) {
    return c;
  } else {
    return EOF;
  }
}
