#include <cpu/ioports.h>
#include <cpu/paging.h>
#include <elf.h>
#include <monocypher.h>
#include <panic.h>
#include <string.h>
#include <struct.h>
#include <sys/lock.h>

extern void jmp_user(uintptr_t, size_t, uintptr_t);
extern int _binary____public_key_start;

void load_elf(size_t file_i) {
  uintptr_t address = loader_struct.files[file_i].address;
  char signature_name[strlen((char*) loader_struct.files[file_i].name) + 5];
  strcpy(signature_name, (char*) loader_struct.files[file_i].name);
  strcat(signature_name, ".sig");
  bool verified = false;
  for (size_t i = 0; i < 64; i++) {
    if (!strcmp((char*) loader_struct.files[i].name, signature_name)) {
      if (crypto_check((uint8_t*) loader_struct.files[i].address, (uint8_t*) &_binary____public_key_start, (uint8_t*) address, loader_struct.files[file_i].size)) {
        panic("Invalid executable signature");
      }
      verified = true;
      break;
    }
  }
  if (!verified) {
    panic("No valid signature found");
  }
  struct elf_header* header = (struct elf_header*) address;
  if (!elf_validate(header)) {
    panic("Executable isn't a valid ELF executable");
  }
  uintptr_t tls = 0;
  size_t tls_size = 0;
  for (size_t i = 0; i < header->npheaders; i++) {
    struct elf_pheader* pheader = (struct elf_pheader*) (address + header->pheader_offset + i * header->pheader_size);
    if (pheader->type != ELF_PHEADER_TYPE_LOAD && pheader->type != ELF_PHEADER_TYPE_TLS) {
      continue;
    }
    uintptr_t mapped_address;
    if (pheader->type == ELF_PHEADER_TYPE_LOAD) {
      mapped_address = pheader->memory_address;
      map_range(pheader->memory_address, pheader->memory_size, false, true, false);
    } else {
      mapped_address = get_free_range(pheader->memory_size, false, true, false, 0x80000000);
      tls = mapped_address;
      tls_size = pheader->memory_size;
    }
    memset((void*) mapped_address, 0, pheader->memory_size);
    memcpy((void*) mapped_address, (void*) address + pheader->file_offset, pheader->file_size);
    if (pheader->type == ELF_PHEADER_TYPE_LOAD) {
      set_paging_flags(mapped_address, pheader->memory_size, true, pheader->flags & ELF_PHEADER_FLAGS_WRITE, pheader->flags & ELF_PHEADER_FLAGS_EXEC);
    } else {
      set_paging_flags(mapped_address, pheader->memory_size, true, false, false);
    }
  }
  if (header->entry >= 0x8000000000000000) {
    panic("Executable is higher half");
  }
  release_lock();
  jmp_user(tls, tls_size, header->entry);
}
