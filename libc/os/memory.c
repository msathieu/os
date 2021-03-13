#include <__/syscall.h>
#include <stddef.h>

uintptr_t map_physical_memory(uintptr_t address, size_t size) {
  return _syscall(_SYSCALL_MAP_PHYS_MEMORY, address, size, 0, 0, 0);
}
