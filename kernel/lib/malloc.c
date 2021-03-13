#include <heap.h>
#include <panic.h>
#include <stdint.h>

extern int malloc_start[];
static uintptr_t malloc_address = (uintptr_t) malloc_start;

static void* early_alloc(size_t size, size_t alignment) {
  malloc_address = (malloc_address + alignment - 1) / alignment * alignment;
  if (malloc_address + size > (uintptr_t) malloc_start + 0x800000) {
    panic("Out of memory in malloc buffer");
  }
  uintptr_t addr = malloc_address;
  malloc_address += size;
  return (void*) addr;
}
void* malloc(size_t size) {
  if (heap_enabled) {
    return heap_alloc(size, 0);
  } else {
    return early_alloc(size, 16);
  }
}
void* valloc(size_t size) {
  if (heap_enabled) {
    return heap_alloc(size, 1);
  } else {
    return early_alloc(size, 0x1000);
  }
}
