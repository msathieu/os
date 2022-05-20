#include <elf.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

int _noremove_args;
int _noenvironment_vars;

static bool fn_read(void* buffer, void* handle, size_t offset, size_t size) {
  fseek(handle, offset, SEEK_SET);
  fread(buffer, size, 1, handle);
  return true;
}
static int get_permissions_bitmask(bool read, bool write, bool exec) {
  int permissions = 0;
  if (read) {
    permissions |= PROT_READ;
  }
  if (write) {
    permissions |= PROT_WRITE;
  }
  if (exec) {
    permissions |= PROT_EXEC;
  }
  return permissions;
}
static uintptr_t fn_mmap(uintptr_t start, size_t size, bool read, bool write, bool exec) {
  return (uintptr_t) mmap((void*) start, size, get_permissions_bitmask(read, write, exec), MAP_ANONYMOUS, 0, 0);
}
static bool fn_protect(uintptr_t start, size_t size, bool read, bool write, bool exec) {
  mprotect((void*) start, size, get_permissions_bitmask(read, write, exec));
  return true;
}
int main(int argc, char* argv[]) {
  if (!argc) {
    return 1;
  }
  FILE* file = fopen(argv[0], "r");
  if (!file) {
    return 1;
  }
  struct elf_header header;
  fread(&header, sizeof(struct elf_header), 1, file);
  if (!elf_validate(&header)) {
    return 1;
  }
  uintptr_t tls = 0;
  size_t tls_size = 0;
  if (!elf_load(&header, fn_read, fn_mmap, fn_protect, file, &tls, &tls_size)) {
    return 1;
  }
  fclose(file);
  asm volatile("mov %0, %%rdi; mov %1, %%rsi; jmp *%2"
               :
               : "r"(tls), "r"(tls_size), "r"(header.entry));
}
