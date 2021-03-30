#include <stdio.h>
#include <string.h>

int fputs(const char* restrict str, FILE* restrict file) {
  fwrite(str, 1, strlen(str), file);
  return 0;
}
