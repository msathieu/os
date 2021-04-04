#include <__/syscall.h>
#include <stdbool.h>
#include <stddef.h>

uintptr_t map_physical_memory(uintptr_t address, size_t size, bool child) {
  return _syscall(_SYSCALL_MAP_PHYS_MEMORY, address, size, child, 0, 0);
}
