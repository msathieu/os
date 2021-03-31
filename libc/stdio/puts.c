#include <stdio.h>
#include <string.h>

int puts(const char* str) {
  int return_value = fputs(str, stdout);
  if (return_value == EOF) {
    return EOF;
  }
  return putchar('\n');
}
