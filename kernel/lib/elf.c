#include <cpu/ioports.h>
#include <cpu/paging.h>
#include <monocypher.h>
#include <panic.h>
#include <string.h>
#include <struct.h>

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

extern void jmp_user(uintptr_t, size_t, uintptr_t);
extern int _binary____public_key_start;

void load_elf(size_t file_i) {
  uintptr_t address = loader_struct.files[file_i].address;
  char signature_name[strlen((char*) loader_struct.files[file_i].name) + 5];
  strcpy(signature_name, (char*) loader_struct.files[file_i].name);
  strcat(signature_name, ".sig");
  bool verified = 0;
  for (size_t i = 0; i < 64; i++) {
    if (!strcmp((char*) loader_struct.files[i].name, signature_name)) {
      if (crypto_check((uint8_t*) loader_struct.files[i].address, (uint8_t*) &_binary____public_key_start, (uint8_t*) address, loader_struct.files[file_i].size)) {
        panic("Invalid executable signature");
      }
      verified = 1;
      break;
    }
  }
  if (!verified) {
    panic("No valid signature found");
  }
  struct elf_header* header = (struct elf_header*) address;
  if (header->magic[0] != ELF_MAGIC || header->magic[1] != 'E' || header->magic[2] != 'L' || header->magic[3] != 'F') {
    panic("Executable isn't a valid ELF executable: invalid magic value");
  }
  if (header->type != ELF_TYPE_EXEC) {
    panic("Executable has unsupported ELF type");
  }
  if (header->architecture != ELF_ARCH_X86_64) {
    panic("Executable isn't built for the correct architecture, needs to be x86_64");
  }
  uintptr_t tls = 0;
  size_t tls_size = 0;
  for (size_t i = 0; i < header->num_pheaders; i++) {
    struct program_header* pheader = (struct program_header*) (address + header->pheader_offset + i * header->pheader_size);
    if (pheader->type != ELF_SEGMENT_LOAD && pheader->type != ELF_SEGMENT_TLS) {
      continue;
    }
    uintptr_t mapped_address;
    if (pheader->type == ELF_SEGMENT_LOAD) {
      mapped_address = pheader->memory_address;
      map_range(pheader->memory_address, pheader->memory_size, 0, 1, 0);
    } else {
      mapped_address = get_free_range(pheader->memory_size, 0, 1, 0, 0x80000000);
      tls = mapped_address;
      tls_size = pheader->memory_size;
    }
    memset((void*) mapped_address, 0, pheader->memory_size);
    memcpy((void*) mapped_address, (void*) address + pheader->file_offset, pheader->file_size);
    if (pheader->type == ELF_SEGMENT_LOAD) {
      set_paging_flags(mapped_address, pheader->memory_size, 1, pheader->flags & ELF_FLAGS_WRITE, pheader->flags & ELF_FLAGS_EXEC);
    } else {
      set_paging_flags(mapped_address, pheader->memory_size, 1, 0, 0);
    }
  }
  if (header->entry >= 0x8000000000000000) {
    panic("Executable is higher half");
  }
  if (loader_struct.ci) {
    outw(0x604, 0x2000);
    while (1)
      ;
  }
  jmp_user(tls, tls_size, header->entry);
}
