#include <panic.h>
#include <stddef.h>
#include <stdint.h>

extern int malloc_start[];
static uintptr_t malloc_address = (uintptr_t) malloc_start;

void* vcalloc(size_t size) {
  malloc_address = (malloc_address + 0xfff) / 0x1000 * 0x1000;
  if (malloc_address + size > (uintptr_t) malloc_start + 0x80000) {
    panic("Out of memory in malloc buffer");
  }
  uintptr_t addr = malloc_address;
  malloc_address += size;
  return (void*) addr;
}
