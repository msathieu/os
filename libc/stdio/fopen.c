#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE* fopen(const char* restrict path, const char* restrict arg_mode) {
  size_t flags = 0;
  if (!strcmp(arg_mode, "r")) {
    flags = O_RDONLY;
  } else if (!strcmp(arg_mode, "w")) {
    flags = O_WRONLY | O_CREAT | O_TRUNC;
  }
  int fd = open(path, flags);
  if (fd != -1) {
    FILE* file = calloc(1, sizeof(FILE));
    file->fd = fd;
    return file;
  } else {
    return 0;
  }
}
