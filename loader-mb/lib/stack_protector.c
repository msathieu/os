#include <panic.h>
#include <stdint.h>

uint32_t __stack_chk_guard = 0x8b40f7bf;

void __stack_chk_fail(void) {
  panic("Stack smashing detected");
}
