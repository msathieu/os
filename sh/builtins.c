#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void builtin_exit(char* return_value, __attribute__((unused)) FILE* _stdout) {
  if (return_value) {
    exit(atoi(return_value));
  } else {
    exit(0);
  }
}
void builtin_export(char* variable, FILE* _stdout) {
  if (!variable) {
    return;
  }
  if (!strcmp(variable, "-p")) {
    for (size_t i = 0; environ[i]; i++) {
      fprintf(_stdout, "export %s\n", environ[i]);
    }
  } else {
    char* value = strchr(variable, '=');
    if (!value) {
      return;
    }
    value[0] = 0;
    setenv(variable, value + 1, 1);
  }
}
