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
  for (size_t i = 0; i < header->npheaders; i++) {
    struct elf_pheader* pheader = (struct elf_pheader*) (addr + (size_t) header->pheader_offset + i * header->pheader_size);
    if (pheader->type != ELF_PHEADER_TYPE_LOAD) {
      continue;
    }
    uintptr_t physical_addr = pheader->memory_address - KERNEL_VIRTUAL_ADDRESS + loader_struct.kernel_physical_address;
    memset((void*) physical_addr, 0, pheader->memory_size);
    memcpy((void*) physical_addr, (void*) addr + pheader->file_offset, pheader->file_size);
  }
  kernel_entry = header->entry;
  if (kernel_entry < 0x8000000000000000) {
    panic("Kernel isn't higher-half");
  }
}
