#include <stdio.h>
#include <string.h>

int puts(const char* str) {
  size_t size = strlen(str);
  char buffer[size + 2];
  strcpy(buffer, str);
  buffer[size] = '\n';
  buffer[size + 1] = 0;
  return fputs(buffer, stdout);
}
