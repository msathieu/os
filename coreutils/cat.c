#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  int exit = 0;
  for (size_t i = 1; i < (size_t) argc; i++) {
    FILE* file = fopen(argv[i], "r");
    if (!file) {
      exit = 1;
      continue;
    }
    while (true) {
      char buffer[1024];
      ssize_t size = read(fileno(file), buffer, 1024);
      if (!size || size == -1) {
        break;
      }
      fwrite(buffer, 1, size, stdout);
    }
    fclose(file);
  }
  return exit;
}
