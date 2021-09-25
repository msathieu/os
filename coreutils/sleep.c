#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  if (argc != 2) {
    puts("Invalid arguments");
    return 1;
  }
  sleep(atol(argv[1]));
}
