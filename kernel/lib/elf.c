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

static bool fn_read(void* buffer, void* handle, size_t offset, size_t size) {
  memcpy(buffer, handle + offset, size);
  return true;
}
static uintptr_t fn_mmap(uintptr_t start, size_t size, __attribute__((unused)) bool read, bool write, bool exec) {
  if (!start) {
    start = 0x80000000;
  }
  map_range(start, size, false, write, exec);
  return start;
}
static bool fn_protect(uintptr_t start, size_t size, __attribute__((unused)) bool read, bool write, bool exec) {
  set_paging_flags(start, size, true, write, exec);
  return true;
}
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
  if (!elf_load(header, fn_read, fn_mmap, fn_protect, (void*) address, &tls, &tls_size)) {
    panic("Couldn't load executable");
  }
  release_lock();
  jmp_user(tls, tls_size, header->entry);
}
