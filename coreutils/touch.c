#include <fcntl.h>
#include <unistd.h>

int main(int argc, char* argv[]) {
  for (size_t i = 1; i < (size_t) argc; i++) {
    int fd = open(argv[i], O_RDONLY | O_CREAT);
    if (fd != -1) {
      close(fd);
    }
  }
}
