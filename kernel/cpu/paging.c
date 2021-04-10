#include <bitset.h>
#include <cpu/isr.h>
#include <cpu/paging.h>
#include <cpuid.h>
#include <panic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <struct.h>
#include <sys/scheduler.h>

struct paging_entry {
  uint64_t present : 1;
  uint64_t write : 1;
  uint64_t user : 1;
  uint64_t write_through : 1;
  uint64_t cache_disable : 1;
  uint64_t accessed : 1;
  uint64_t dirty : 1;  // Only for pages
  uint64_t pat : 1;    // Only for pages
  uint64_t global : 1; // Only for pages
  uint64_t reserved : 3;
  uint64_t address : 38;
  uint64_t reserved2 : 13;
  uint64_t noexec : 1;
};
struct paging_table {
  struct paging_entry phys_entries[512];
  struct paging_table* entries[512];
};

struct paging_table* current_pml4;
static bool paging_enabled;
static uint64_t* frames;
static size_t nframes;
static uintptr_t physical_mappings_addr = PAGING_PHYSICAL_MAPPINGS_START;
struct process* physical_mappings_process;
struct task* physical_mappings_task;
extern int text_start[];
extern int text_end[];
extern int rodata_start[];
extern int rodata_end[];
extern int data_start[];
extern int data_end[];
extern int bss_start[];
extern int bss_end[];

static size_t find_free_frame(void) {
  for (size_t i = 0; i < nframes / 64; i++) {
    if (frames[i] == 0xffffffffffffffff) {
      continue;
    }
    for (size_t j = 0; j < 64; j++) {
      if (!bitset_test(frames, i * 64 + j)) {
        return i * 64 + j;
      }
    }
  }
  panic("Out of free memory");
}
static inline void invlpg(uintptr_t address) {
  asm volatile("invlpg (%0)"
               :
               : "r"(address));
}
static struct paging_entry* get_page(uintptr_t address, struct paging_table* pml4) {
  address /= 0x1000;
  if (!address) {
    panic("Tried to obtain first page");
  }
  size_t page_table_i = address % 512;
  size_t page_directory_i = address / 512 % 512;
  size_t pdpt_i = address / 512 / 512 % 512;
  size_t pml4_i = address / 512 / 512 / 512 % 512;
  if (pml4->entries[pml4_i]) {
    struct paging_table* pdpt = pml4->entries[pml4_i];
    if (pdpt->entries[pdpt_i]) {
      struct paging_table* page_directory = pdpt->entries[pdpt_i];
      if (page_directory->entries[page_directory_i]) {
        struct paging_table* page_table = page_directory->entries[page_directory_i];
        struct paging_entry* page = &page_table->phys_entries[page_table_i];
        if (page->present) {
          return page;
        }
      }
    }
  }
  return 0;
}
uintptr_t convert_to_physical(uintptr_t address, struct paging_table* pml4) {
  if (!paging_enabled) {
    return address - KERNEL_VIRTUAL_ADDRESS + loader_struct.kernel_physical_addr;
  }
  struct paging_entry* page = get_page(address, pml4);
  if (page) {
    return page->address * 0x1000 + address % 0x1000;
  } else {
    panic("Invalid virtual address");
  }
}
static void gen_entry(struct paging_table* table, size_t i) {
  if (table->entries[i]) {
    return;
  }
  table->entries[i] = valloc(sizeof(struct paging_table));
  memset(table->entries[i], 0, sizeof(struct paging_table));
  table->phys_entries[i].present = 1;
  table->phys_entries[i].user = 1;
  table->phys_entries[i].write = 1;
  table->phys_entries[i].address = convert_to_physical((uintptr_t) table->entries[i], current_pml4) / 0x1000;
}
bool is_page_mapped(uintptr_t address, bool write) {
  struct paging_entry* page = get_page(address, current_pml4);
  if (page && (page->write || !write)) {
    return 1;
  } else {
    return 0;
  }
}
// Doesn't free frame
void unmap_page(uintptr_t address) {
  struct paging_entry* page = get_page(address, current_pml4);
  if (page) {
    page->present = 0;
    invlpg(address);
  } else {
    panic("Requested page doesn't exist");
  }
}
void free_page(uintptr_t address) {
  struct paging_entry* page = get_page(address, current_pml4);
  if (page) {
    page->present = 0;
    invlpg(address);
    bitset_clear(frames, page->address);
  } else {
    panic("Requested page doesn't exist");
  }
}
void create_mapping(uintptr_t virt, uintptr_t phys, bool user, bool write, bool exec, bool cache_disable) {
  virt /= 0x1000;
  if (!virt) {
    panic("Tried to map first page");
  }
  size_t page_table_i = virt % 512;
  size_t page_directory_i = virt / 512 % 512;
  size_t pdpt_i = virt / 512 / 512 % 512;
  size_t pml4_i = virt / 512 / 512 / 512 % 512;
  gen_entry(current_pml4, pml4_i);
  struct paging_table* pdpt = current_pml4->entries[pml4_i];
  gen_entry(pdpt, pdpt_i);
  struct paging_table* page_directory = pdpt->entries[pdpt_i];
  gen_entry(page_directory, page_directory_i);
  struct paging_table* page_table = page_directory->entries[page_directory_i];
  struct paging_entry* page = &page_table->phys_entries[page_table_i];
  page->present = 1;
  page->user = user;
  page->write = write;
  page->noexec = !exec;
  page->cache_disable = cache_disable;
  if (pml4_i >= 256) {
    page->global = 1;
  }
  page->address = phys / 0x1000;
  bitset_set(frames, page->address);
}
static void map_address(uintptr_t virtual_address, bool user, bool write, bool exec) {
  uintptr_t physical_address = find_free_frame() * 0x1000;
  create_mapping(virtual_address, physical_address, user, write, exec, 0);
}
void map_range(uintptr_t virtual_address, size_t size, bool user, bool write, bool exec) {
  uintptr_t end = virtual_address + size;
  virtual_address = virtual_address / 0x1000 * 0x1000;
  end = (end + 0xfff) / 0x1000 * 0x1000;
  for (size_t address = virtual_address; address < end; address += 0x1000) {
    map_address(address, user, write, exec);
  }
}
void set_paging_flags(uintptr_t address, size_t size, bool user, bool write, bool exec) {
  uintptr_t end = address + size;
  address = address / 0x1000 * 0x1000;
  end = (end + 0xfff) / 0x1000 * 0x1000;
  for (size_t virtual_address = address; virtual_address < end; virtual_address += 0x1000) {
    uintptr_t physical_address = convert_to_physical(virtual_address, current_pml4);
    create_mapping(virtual_address, physical_address, user, write, exec, 0);
    invlpg(virtual_address);
  }
}
void* map_physical(uintptr_t physical_address, size_t size, bool write, bool cache_disable) {
  uintptr_t offset = physical_address % 0x1000;
  uintptr_t end = physical_address + size;
  physical_address = physical_address / 0x1000 * 0x1000;
  end = (end + 0xfff) / 0x1000 * 0x1000;
  uintptr_t virtual_address = physical_mappings_addr;
  physical_mappings_addr += end - physical_address;
  if (physical_mappings_addr > PAGING_PHYSICAL_MAPPINGS_START + PAGING_PHYSICAL_MAPPINGS_SIZE) {
    panic("Out of memory reserved for physical mappings");
  }
  for (size_t i = 0; i < end - physical_address; i += 0x1000) {
    create_mapping(virtual_address + i, physical_address + i, 0, write, 0, cache_disable);
  }
  return (void*) virtual_address + offset;
}
struct paging_table* create_pml4(void) {
  struct paging_table* pml4 = valloc(sizeof(struct paging_table));
  memset(pml4, 0, sizeof(struct paging_table));
  for (size_t i = 256; i < 512; i++) {
    pml4->entries[i] = current_pml4->entries[i];
    pml4->phys_entries[i] = current_pml4->phys_entries[i];
  }
  return pml4;
}
void destroy_pml4(struct paging_table* pml4) {
  for (size_t i = 0; i < 256; i++) {
    struct paging_table* pdpt = pml4->entries[i];
    if (pdpt) {

      for (size_t j = 0; j < 512; j++) {
        struct paging_table* page_directory = pdpt->entries[j];
        if (page_directory) {

          for (size_t k = 0; k < 512; k++) {
            struct paging_table* page_table = page_directory->entries[k];
            if (page_table) {

              for (size_t l = 0; l < 512; l++) {
                if (page_table->phys_entries[l].present && (uintptr_t) page_table->phys_entries[l].address * 0x1000 < PAGING_USER_PHYS_MAPPINGS_START) {
                  bitset_clear(frames, page_table->phys_entries[l].address);
                }
              }

              free(page_table);
            }
          }

          free(page_directory);
        }
      }

      free(pdpt);
    }
  }
  free(pml4);
}
void switch_pml4(struct paging_table* pml4) {
  asm volatile("mov %0, %%cr3"
               :
               : "r"(convert_to_physical((uintptr_t) pml4, current_pml4)));
  current_pml4 = pml4;
}
uintptr_t get_free_range(size_t size, bool write, bool exec, uintptr_t requested_start) {
  size = (size + 0xfff) / 0x1000 * 0x1000;
  uintptr_t start = 0;
  for (uintptr_t i = requested_start; i < PAGING_USER_PHYS_MAPPINGS_START; i += 0x1000) {
    if (!start) {
      if (!get_page(i, current_pml4)) {
        start = i;
      } else {
        continue;
      }
    } else {
      if (get_page(i, current_pml4)) {
        start = 0;
        continue;
      }
    }
    if (i + 0x1000 - start == size) {
      map_range(start, size, 1, write, exec);
      return start;
    }
  }
  return 0;
}
uintptr_t get_free_ipc_range(size_t size) {
  long start = -1;
  for (size_t i = 0; i < PAGING_PHYSICAL_MAPPINGS_SIZE; i += 0x1000) {
    if (start == -1) {
      if (!bitset_test(physical_mappings_process->mappings_bitset, i / 0x1000)) {
        start = i;
      } else {
        continue;
      }
    } else {
      if (bitset_test(physical_mappings_process->mappings_bitset, i / 0x1000)) {
        start = -1;
        continue;
      }
    }
    if (i + 0x1000 - start == size) {
      for (size_t j = start; j <= i; j += 0x1000) {
        bitset_set(physical_mappings_process->mappings_bitset, j / 0x1000);
        if (physical_mappings_task) {
          bitset_set(physical_mappings_task->mappings_bitset, j / 0x1000);
        }
      }
      return PAGING_USER_PHYS_MAPPINGS_START + start;
    }
  }
  return 0;
}
static void fault_handler(struct isr_registers* registers) {
  printf("Page fault at %lx: ", registers->rip);
  if (!(registers->error & 1)) {
    printf("not present ");
  }
  if (registers->error & 2) {
    printf("write ");
  }
  if (registers->error & 4) {
    printf("user ");
  }
  if (registers->error & 16) {
    printf("execute");
  }
  printf("\n");
  if (registers->cs == 0x23) {
    terminate_current_task(registers);
  } else {
    panic("Page fault occurred");
  }
}
void setup_paging(void) {
  for (size_t i = 0; i < 1024; i++) {
    size_t _nframes = (loader_struct.memory_map[i].address + loader_struct.memory_map[i].size) / 0x1000;
    if (_nframes > nframes) {
      nframes = _nframes;
    }
  }
  frames = malloc((nframes + 63) / 64 * 8);
  memset(frames, 0xff, (nframes + 63) / 64 * 8);
  for (size_t i = 0; i < 1024; i++) {
    for (size_t j = 0; j < loader_struct.memory_map[i].size; j += 0x1000) {
      bitset_clear(frames, (loader_struct.memory_map[i].address + j) / 0x1000);
    }
  }
  if (bitset_test(frames, 1)) {
    panic("Second page is already mapped");
  }
  bitset_set(frames, 1);
  current_pml4 = valloc(sizeof(struct paging_table));
  for (uintptr_t virtual_addr = (uintptr_t) text_start; virtual_addr < (uintptr_t) text_end; virtual_addr += 0x1000) {
    create_mapping(virtual_addr, virtual_addr - KERNEL_VIRTUAL_ADDRESS + loader_struct.kernel_physical_addr, 0, 0, 1, 0);
  }
  for (uintptr_t virtual_addr = (uintptr_t) rodata_start; virtual_addr < (uintptr_t) rodata_end; virtual_addr += 0x1000) {
    create_mapping(virtual_addr, virtual_addr - KERNEL_VIRTUAL_ADDRESS + loader_struct.kernel_physical_addr, 0, 0, 0, 0);
  }
  for (uintptr_t virtual_addr = (uintptr_t) data_start; virtual_addr < (uintptr_t) data_end; virtual_addr += 0x1000) {
    create_mapping(virtual_addr, virtual_addr - KERNEL_VIRTUAL_ADDRESS + loader_struct.kernel_physical_addr, 0, 1, 0, 0);
  }
  for (uintptr_t virtual_addr = (uintptr_t) bss_start; virtual_addr < (uintptr_t) bss_end; virtual_addr += 0x1000) {
    create_mapping(virtual_addr, virtual_addr - KERNEL_VIRTUAL_ADDRESS + loader_struct.kernel_physical_addr, 0, 1, 0, 0);
  }
  for (size_t i = 0; i < 64; i++) {
    loader_struct.files[i].address = (uintptr_t) map_physical(loader_struct.files[i].address, loader_struct.files[i].size, 0, 0);
  }
  for (size_t i = 256; i < 512; i++) {
    gen_entry(current_pml4, i);
    current_pml4->phys_entries[i].user = 0;
    if (i != 511) {
      current_pml4->phys_entries[i].noexec = 1;
    }
  }
  asm volatile("rdmsr; bts $11, %%rax; wrmsr" // Execute disable
               :
               : "c"(0xc0000080)
               : "rax", "rdx");
  switch_pml4(current_pml4);
  paging_enabled = 1;
  isr_handlers[14] = fault_handler;
}
