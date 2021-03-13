#include <stdlib.h>
#include <string.h>

char* strdup(const char* str) {
  char* new_str = malloc(strlen(str) + 1);
  strcpy(new_str, str);
  return new_str;
}
