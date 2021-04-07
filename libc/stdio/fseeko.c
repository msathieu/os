#include <stdio.h>

int fseeko(FILE* file, off_t position, int mode) {
  return fseek(file, position, mode);
}
