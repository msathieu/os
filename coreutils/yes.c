#include <stdbool.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
  while (true) {
    if (argc == 2) {
      puts(argv[1]);
    } else {
      puts("y");
    }
  }
}
