#include <stdio.h>
#include <unistd.h>

size_t fwrite(const void* restrict buffer, size_t size, size_t num, FILE* restrict file) {
  return write(file->fd, buffer, size * num) / size;
}
