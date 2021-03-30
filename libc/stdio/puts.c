#include <stdio.h>
#include <string.h>

int puts(const char* str) {
  fputs(str, stdout);
  putchar('\n');
  return 0;
}
