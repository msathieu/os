#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#define ELF_MAGIC 0x7f
#define ELF_TYPE_EXEC 2
#define ELF_ARCH_X86_64 0x3e
struct elf_header {
  uint8_t magic[4];
  uint8_t bits;
  uint8_t endianness;
  uint8_t header_version;
  uint8_t abi;
  uint64_t unused;
  uint16_t type;
  uint16_t architecture;
  uint32_t version;
  uint64_t entry;
  uint64_t pheader_offset;
  uint64_t sheader_offset;
  uint32_t flags;
  uint16_t header_size;
  uint16_t pheader_size;
  uint16_t num_pheaders;
};
#define ELF_SEGMENT_LOAD 1
#define ELF_SEGMENT_TLS 7
#define ELF_FLAGS_EXEC 1
#define ELF_FLAGS_WRITE 2
struct program_header {
  uint32_t type;
  uint32_t flags;
  uint64_t file_offset;
  uint64_t memory_address;
  uint64_t unused;
  uint64_t file_size;
  uint64_t memory_size;
};

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
  if (header.magic[0] != ELF_MAGIC || header.magic[1] != 'E' || header.magic[2] != 'L' || header.magic[3] != 'F') {
    return 1;
  }
  if (header.type != ELF_TYPE_EXEC) {
    return 1;
  }
  if (header.architecture != ELF_ARCH_X86_64) {
    return 1;
  }
  uintptr_t tls = 0;
  size_t tls_size = 0;
  for (size_t i = 0; i < header.num_pheaders; i++) {
    fseek(file, header.pheader_offset + i * header.pheader_size, SEEK_SET);
    struct program_header pheader;
    fread(&pheader, sizeof(struct program_header), 1, file);
    if (pheader.type != ELF_SEGMENT_LOAD && pheader.type != ELF_SEGMENT_TLS) {
      continue;
    }
    void* mapped_address;
    if (pheader.type == ELF_SEGMENT_LOAD) {
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
    if (pheader.type == ELF_SEGMENT_LOAD) {
      if (pheader.flags & ELF_FLAGS_WRITE) {
        permissions |= PROT_WRITE;
      }
      if (pheader.flags & ELF_FLAGS_EXEC) {
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
