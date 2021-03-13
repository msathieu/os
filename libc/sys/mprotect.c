#include <__/syscall.h>
#include <stdbool.h>
#include <sys/mman.h>

int mprotect(void* address, size_t size, int prot) {
  size = (size + 0xfff) / 0x1000 * 0x1000;
  _syscall(_SYSCALL_CHANGE_MEMORY_PERMISSIONS, (uintptr_t) address, size, (bool) (prot & PROT_WRITE), (bool) (prot & PROT_EXEC), 0);
  return 0;
}
