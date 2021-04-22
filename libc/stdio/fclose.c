#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int fclose(FILE* file) {
  int return_value = close(file->fd);
  free(file);
  if (return_value) {
    return EOF;
  } else {
    return 0;
  }
}
