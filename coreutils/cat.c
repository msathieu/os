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
    while (1) {
      char buffer[1024];
      size_t size = read(fileno(file), buffer, 1024);
      if (!size) {
        break;
      }
      fwrite(buffer, 1, size, stdout);
    }
    fclose(file);
  }
  return exit;
}
