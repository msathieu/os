#include <stdio.h>
#include <string.h>

int fputs(const char* restrict str, FILE* restrict file) {
  size_t len = strlen(str);
  size_t return_value = fwrite(str, 1, len, file);
  if (return_value == len) {
    return 0;
  } else {
    return EOF;
  }
}
