#include <stdio.h>
#include <string.h>

int puts(const char* str) {
  fwrite(str, 1, strlen(str), stdout);
  putchar('\n');
  return 0;
}
