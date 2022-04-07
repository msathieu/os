#include <stdbool.h>

void lmain(void) {
  while (true) {
    asm volatile("hlt");
  }
}
