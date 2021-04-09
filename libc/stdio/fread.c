#include <stdio.h>
#include <unistd.h>

size_t fread(void* restrict buffer, size_t size, size_t num, FILE* restrict file) {
  ssize_t return_value = read(file->fd, buffer, size * num) / size;
  if (return_value < 0) {
    return 0;
  } else {
    return return_value;
  }
}
