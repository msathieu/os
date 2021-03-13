#include <stdint.h>
#include <stdlib.h>

uint64_t __stack_chk_guard;

void __stack_chk_fail(void) {
  exit(1);
}
