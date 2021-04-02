#include <stdio.h>

int main(int argc, char* argv[]) {
  for (size_t i = 1; i < (size_t) argc; i++) {
    FILE* file = fopen(argv[i], "r");
    if (!file) {
      file = fopen(argv[i], "w");
    }
    if (file) {
      fclose(file);
    }
  }
}
