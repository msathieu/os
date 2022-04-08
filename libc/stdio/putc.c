#include <stdio.h>

int putc(int c, FILE* file) {
  return fputc(c, file);
}
