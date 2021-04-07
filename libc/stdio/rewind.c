#include <stdio.h>

void rewind(FILE* file) {
  fseek(file, 0, SEEK_SET);
}
