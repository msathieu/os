#include <stdio.h>

int main(int argc, char* argv[]) {
  while (1) {
    if (argc == 2) {
      puts(argv[1]);
    } else {
      puts("y");
    }
  }
}
