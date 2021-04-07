#include <stdio.h>

off_t ftello(FILE* file) {
  return ftell(file);
}
