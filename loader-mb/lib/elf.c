#include <monocypher.h>
#include <multiboot.h>
#include <panic.h>
#include <stdbool.h>
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
struct program_header {
  uint32_t type;
  uint32_t flags;
  uint64_t file_offset;
  uint64_t memory_address;
  uint64_t unused;
  uint64_t file_size;
  uint64_t memory_size;
};

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
  if (header->magic[0] != ELF_MAGIC || header->magic[1] != 'E' || header->magic[2] != 'L' || header->magic[3] != 'F') {
    panic("Kernel isn't a valid ELF executable: invalid magic value");
  }
  if (header->type != ELF_TYPE_EXEC) {
    panic("Kernel has unsupported ELF type");
  }
  if (header->architecture != ELF_ARCH_X86_64) {
    panic("Kernel isn't built for the correct architecture, needs to be x86_64");
  }
  for (size_t i = 0; i < header->num_pheaders; i++) {
    struct program_header* pheader = (struct program_header*) (addr + header->pheader_offset + i * header->pheader_size);
    if (pheader->type != ELF_SEGMENT_LOAD) {
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
  loader_struct.kernel_physical_addr = physical_base;
  for (size_t i = 0; i < header->num_pheaders; i++) {
    struct program_header* pheader = (struct program_header*) (addr + (size_t) header->pheader_offset + i * header->pheader_size);
    if (pheader->type != ELF_SEGMENT_LOAD) {
      continue;
    }
    uintptr_t physical_addr = pheader->memory_address - KERNEL_VIRTUAL_ADDRESS + loader_struct.kernel_physical_addr;
    memset((void*) physical_addr, 0, pheader->memory_size);
    memcpy((void*) physical_addr, (void*) addr + pheader->file_offset, pheader->file_size);
  }
  kernel_entry = header->entry;
  if (kernel_entry < 0x8000000000000000) {
    panic("Kernel isn't higher-half");
  }
}
