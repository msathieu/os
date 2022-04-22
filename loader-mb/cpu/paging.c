#include <elf.h>
#include <multiboot.h>
#include <stdbool.h>
#include <stdlib.h>
#include <struct.h>

struct paging_entry {
  uint64_t present : 1;
  uint64_t write : 1;
  uint64_t user : 1;
  uint64_t write_through : 1;
  uint64_t cache_disable : 1;
  uint64_t accessed : 1;
  uint64_t reserved : 6;
  uint64_t address : 38;
  uint64_t reserved2 : 13;
  uint64_t noexec : 1;
};
struct paging_table {
  struct paging_entry phys_entries[512];
  struct paging_table* entries[512];
};

_Alignas(0x1000) static struct paging_table pml4;

static void gen_entry(struct paging_table* table, size_t i) {
  if (table->entries[i]) {
    return;
  }
  table->entries[i] = vcalloc(sizeof(struct paging_table));
  table->phys_entries[i].present = true;
  table->phys_entries[i].write = true;
  table->phys_entries[i].address = (uintptr_t) table->entries[i] / 0x1000;
}
static void create_mapping(uint64_t virt, uintptr_t phys) {
  virt /= 0x1000;
  size_t page_table_i = virt % 512;
  size_t page_directory_i = virt / 512 % 512;
  size_t pdpt_i = virt / 512 / 512 % 512;
  size_t pml4_i = virt / 512 / 512 / 512 % 512;
  gen_entry(&pml4, pml4_i);
  struct paging_table* pdpt = pml4.entries[pml4_i];
  gen_entry(pdpt, pdpt_i);
  struct paging_table* page_directory = pdpt->entries[pdpt_i];
  gen_entry(page_directory, page_directory_i);
  struct paging_table* page_table = page_directory->entries[page_directory_i];
  struct paging_entry* page = &page_table->phys_entries[page_table_i];
  page->present = true;
  page->write = true;
  page->address = phys / 0x1000;
}
static void identity_map(uintptr_t start, uintptr_t end) {
  start = start / 0x1000 * 0x1000;
  for (uintptr_t addr = start; addr < end; addr += 0x1000) {
    create_mapping(addr, addr);
  }
}
void setup_paging(void) {
  identity_map(0, modules_end_addr);
  asm volatile("mov %%cr4, %%eax; bts $5, %%eax; mov %%eax, %%cr4" // Physical address extension
               :
               :
               : "eax");
  asm volatile("rdmsr; bts $8, %%eax; wrmsr" // Long mode enable
               :
               : "c"(0xc0000080)
               : "eax", "edx");
  asm volatile("mov %0, %%cr3"
               :
               : "r"(&pml4));
  asm volatile("mov %%cr0, %%eax; bts $31, %%eax; mov %%eax, %%cr0" // Paging
               :
               :
               : "eax");
  for (uintptr_t addr = 0; addr < kernel_size; addr += 0x1000) {
    create_mapping(KERNEL_VIRTUAL_ADDRESS + addr, loader_struct.kernel_physical_address + addr);
  }
}
