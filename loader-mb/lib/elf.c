#include <elf.h>
#include <monocypher.h>
#include <multiboot.h>
#include <panic.h>
#include <stdbool.h>
#include <string.h>
#include <struct.h>

size_t kernel_size;
uint64_t kernel_entry;
extern int _binary____public_key_start;

static bool fn_read(void* buffer, void* handle, size_t offset, size_t size) {
  memcpy(buffer, handle + offset, size);
  return true;
}
static uintptr_t fn_mmap(uintptr_t start, __attribute__((unused)) size_t size, __attribute__((unused)) bool read, __attribute__((unused)) bool write, __attribute__((unused)) bool exec) {
  return start - KERNEL_VIRTUAL_ADDRESS + loader_struct.kernel_physical_address;
}
static bool fn_protect(__attribute__((unused)) uintptr_t start, __attribute__((unused)) size_t size, __attribute__((unused)) bool read, __attribute__((unused)) bool write, __attribute__((unused)) bool exec) {
  return true;
}
void load_kernel(uintptr_t addr, uintptr_t size) {
  bool verified = false;
  for (size_t i = 0; i < 64; i++) {
    if (!strcmp((char*) loader_struct.files[i].name, "kernel.sig")) {
      if (crypto_check((uint8_t*) loader_struct.files[i].address, (uint8_t*) &_binary____public_key_start, (uint8_t*) addr, size)) {
        panic("Invalid kernel signature");
      }
      verified = true;
      break;
    }
  }
  if (!verified) {
    panic("No valid signature found");
  }
  struct elf_header* header = (struct elf_header*) addr;
  if (!elf_validate(header)) {
    panic("Kernel isn't a valid ELF executable");
  }
  for (size_t i = 0; i < header->npheaders; i++) {
    struct elf_pheader* pheader = (struct elf_pheader*) (addr + header->pheader_offset + i * header->pheader_size);
    if (pheader->type != ELF_PHEADER_TYPE_LOAD) {
      continue;
    }
    size_t last_address = pheader->memory_address - KERNEL_VIRTUAL_ADDRESS + pheader->memory_size;
    if (kernel_size < last_address) {
      kernel_size = last_address;
    }
  }
  uintptr_t physical_base = 0;
  for (size_t i = 0; i < 1024; i++) {
    if (loader_struct.memory_map[i].address >= 0x100000000 || loader_struct.memory_map[i].address + loader_struct.memory_map[i].size >= 0x100000000) {
      continue;
    }
    uint64_t start_address = loader_struct.memory_map[i].address;
    uint64_t memory_size = loader_struct.memory_map[i].size;
    if (start_address < modules_end_addr) {
      start_address = (modules_end_addr + 0xfff) / 0x1000 * 0x1000;
      if (start_address - loader_struct.memory_map[i].address > memory_size) {
        continue;
      }
      memory_size -= start_address - loader_struct.memory_map[i].address;
    }
    if (memory_size >= kernel_size) {
      physical_base = start_address;
      break;
    }
  }
  if (!physical_base) {
    panic("Couldn't find big enough memory segment to load kernel");
  }
  loader_struct.kernel_physical_address = physical_base;
  if (!elf_load(header, fn_read, fn_mmap, fn_protect, (void*) addr, 0, 0)) {
    panic("Couldn't load kernel");
  }
  kernel_entry = header->entry;
}
