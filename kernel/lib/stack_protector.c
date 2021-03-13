#include <panic.h>
#include <stdint.h>

uint64_t __stack_chk_guard;

void __stack_chk_fail(void) {
  panic("Stack smashing detected");
}
