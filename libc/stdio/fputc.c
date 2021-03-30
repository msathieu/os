#include <stdio.h>

int fputc(int c, FILE* file) {
  unsigned char arg = c;
  fwrite(&arg, 1, 1, file);
  return c;
}
