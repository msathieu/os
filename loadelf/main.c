#include <elf.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

int _noremove_args;
int _noenvironment_vars;

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
  for (size_t i = 0; i < header.npheaders; i++) {
    fseek(file, header.pheader_offset + i * header.pheader_size, SEEK_SET);
    struct elf_pheader pheader;
    fread(&pheader, sizeof(struct elf_pheader), 1, file);
    if (pheader.type != ELF_PHEADER_TYPE_LOAD && pheader.type != ELF_PHEADER_TYPE_TLS) {
      continue;
    }
    void* mapped_address;
    if (pheader.type == ELF_PHEADER_TYPE_LOAD) {
      mapped_address = mmap((void*) (pheader.memory_address / 0x1000 * 0x1000), pheader.memory_size + pheader.memory_address % 0x1000, PROT_WRITE, MAP_ANONYMOUS, 0, 0);
      if ((uintptr_t) mapped_address != pheader.memory_address / 0x1000 * 0x1000) {
        return 1;
      }
    } else {
      mapped_address = mmap(0, pheader.memory_size, PROT_WRITE, MAP_ANONYMOUS, 0, 0);
      tls = (uintptr_t) mapped_address;
      tls_size = pheader.memory_size;
      pheader.memory_address = tls;
    }
    memset((void*) pheader.memory_address, 0, pheader.memory_size);
    fseek(file, pheader.file_offset, SEEK_SET);
    fread((void*) pheader.memory_address, 1, pheader.file_size, file);
    int permissions = 0;
    if (pheader.type == ELF_PHEADER_TYPE_LOAD) {
      if (pheader.flags & ELF_PHEADER_FLAGS_WRITE) {
        permissions |= PROT_WRITE;
      }
      if (pheader.flags & ELF_PHEADER_FLAGS_EXEC) {
        permissions |= PROT_EXEC;
      }
    }
    mprotect(mapped_address, pheader.memory_size + pheader.memory_address % 0x1000, permissions);
  }
  fclose(file);
  asm volatile("mov %0, %%rdi; mov %1, %%rsi; jmp *%2"
               :
               : "r"(tls), "r"(tls_size), "r"(header.entry));
}
