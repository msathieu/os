#include <stdio.h>

int main(int argc, char* argv[]) {
  for (size_t i = 1; i < (size_t) argc; i++) {
    if (i != 1) {
      putchar(' ');
    }
    printf("%s", argv[i]);
  }
  putchar('\n');
}
