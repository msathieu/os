#include <__/syscall.h>
#include <stdbool.h>
#include <sys/mman.h>

void* mmap(void* address, size_t size, int prot, int flags, __attribute__((unused)) int file, __attribute__((unused)) off_t offset) {
  if (flags & MAP_FIXED || flags & MAP_SHARED || !(flags & MAP_ANONYMOUS)) {
    return MAP_FAILED;
  }
  if (!address) {
    address = (void*) 0x80000000;
  }
  return (void*) _syscall(_SYSCALL_MAP_MEMORY, size, (bool) (prot & PROT_WRITE), (bool) (prot & PROT_EXEC), (uintptr_t) address, 0);
}
