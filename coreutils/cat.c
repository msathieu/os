#include <stdio.h>

int main(int argc, char* argv[]) {
  int exit = 0;
  for (size_t i = 1; i < (size_t) argc; i++) {
    FILE* file = fopen(argv[i], "r");
    if (!file) {
      exit = 1;
      continue;
    }
    while (1) {
      int c = getc(file);
      if (c == EOF) {
        break;
      }
      putchar(c);
    }
    fclose(file);
  }
  return exit;
}
