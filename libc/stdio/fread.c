#include <stdio.h>
#include <unistd.h>

size_t fread(void* restrict buffer, size_t size, size_t num, FILE* restrict file) {
  return read(file->fd, buffer, size * num) / size;
}
