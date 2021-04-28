#include <cpu/paging.h>
#include <cpu/rdrand.h>
#include <heap.h>
#include <panic.h>
#include <sorted_list.h>
#include <stdlib.h>
#include <struct.h>
#define HEAP_START (PAGING_PHYSICAL_MAPPINGS_START + PAGING_PHYSICAL_MAPPINGS_SIZE)
#define HEAP_INITIAL_SIZE 0x800000

struct heap_header {
  struct linked_list_member list_member;
  size_t size;
  bool free;
  long magic;
};
struct heap_footer {
  long magic;
  struct heap_header* header;
};

static long heap_magic;
static struct sorted_list heap_list;
bool heap_enabled;
static size_t heap_size = HEAP_INITIAL_SIZE;

static int heap_compare(void* header1, void* header2) {
  if (((struct heap_header*) header1)->size <= ((struct heap_header*) header2)->size) {
    return -1;
  } else {
    return 1;
  }
}
// Prevent page-aligned allocation to be first and prevent first allocation being freed
__attribute__((optnone)) void placeholder_alloc(void) {
  heap_alloc(8, 0);
}
__attribute__((optnone)) void test_alloc(void) {
  void* alloc_1 = heap_alloc(8, 0);
  void* alloc_2 = heap_alloc(16, 0);
  void* alloc_3 = heap_alloc(8, 0);
  free(alloc_2);
  free(alloc_1);
  free(alloc_3);
  void* alloc_test = heap_alloc(128, 0);
  if (alloc_1 != alloc_test) {
    panic("Memory leak in heap");
  }
  void* alloc_aligned_1 = heap_alloc(8, 1);
  free(alloc_aligned_1);
  if ((uintptr_t) alloc_aligned_1 % 0x1000) {
    panic("valloc doesn't properly align");
  }
  void* alloc_aligned_2 = heap_alloc(8, 1);
  free(alloc_aligned_2);
  if (alloc_aligned_1 != alloc_aligned_2) {
    panic("Memory leak in heap");
  }
  free(alloc_test);
  void* alloc_test_2 = heap_alloc(256, 0);
  free(alloc_test_2);
  if (alloc_test != alloc_test_2) {
    panic("Memory leak in heap");
  }
}
void setup_heap(void) {
  heap_magic = rdrand();
  heap_list.compare = (sorted_list_compare) heap_compare;
  map_range(HEAP_START, HEAP_INITIAL_SIZE, 0, 1, 0);
  struct heap_header* initial_header = (struct heap_header*) HEAP_START;
  initial_header->size = HEAP_INITIAL_SIZE - sizeof(struct heap_header) - sizeof(struct heap_footer);
  initial_header->free = 1;
  initial_header->magic = heap_magic;
  struct heap_footer* initial_footer = (struct heap_footer*) (HEAP_START + sizeof(struct heap_header) + initial_header->size);
  initial_footer->magic = heap_magic;
  initial_footer->header = initial_header;
  insert_sorted_list(&heap_list, &initial_header->list_member);
  heap_enabled = 1;
  placeholder_alloc();
  if (loader_struct.ci) {
    test_alloc();
  }
}
void* heap_alloc(size_t requested_size, bool align) {
  requested_size = (requested_size + 15) / 16 * 16;
  if (align) {
    requested_size += 0x1000;
  }
  struct heap_header* header = 0;
  for (struct heap_header* header_i = (struct heap_header*) heap_list.first; header_i; header_i = (struct heap_header*) header_i->list_member.next) {
    if (header_i->magic != heap_magic) {
      panic("Invalid magic value");
    }
    if (header_i->size >= requested_size) {
      header = header_i;
      break;
    }
  }
  if (!header) {
    panic("Out of heap memory");
  }
  remove_sorted_list(&heap_list, &header->list_member);
  header->free = 0;
  if (header->size - requested_size > sizeof(struct heap_header) + sizeof(struct heap_footer)) {
    size_t new_free_size = header->size - requested_size - sizeof(struct heap_header) - sizeof(struct heap_footer);
    header->size = requested_size;
    struct heap_footer* allocated_footer = (struct heap_footer*) ((uintptr_t) header + sizeof(struct heap_header) + requested_size);
    allocated_footer->magic = heap_magic;
    allocated_footer->header = header;
    struct heap_header* new_free_header = (struct heap_header*) (allocated_footer + 1);
    new_free_header->size = new_free_size;
    new_free_header->free = 1;
    new_free_header->magic = heap_magic;
    struct heap_footer* new_free_footer = (struct heap_footer*) ((uintptr_t) new_free_header + sizeof(struct heap_header) + new_free_size);
    if (new_free_footer->magic != heap_magic) {
      panic("Invalid magic value");
    }
    new_free_footer->header = new_free_header;
    insert_sorted_list(&heap_list, &new_free_header->list_member);
  }
  if (align) {
    // Should never be first allocation
    uintptr_t original_addr = (uintptr_t) header + sizeof(struct heap_header);
    size_t original_size = header->size;
    struct heap_footer* original_previous_footer = (struct heap_footer*) ((uintptr_t) header - sizeof(struct heap_footer));
    if (original_previous_footer->magic != heap_magic) {
      panic("Invalid magic value");
    }
    struct heap_header* previous_header = original_previous_footer->header;
    if (previous_header->magic != heap_magic) {
      panic("Invalid magic value");
    }
    uintptr_t aligned_addr = (original_addr + 0xfff) / 0x1000 * 0x1000;
    header = (struct heap_header*) (aligned_addr - sizeof(struct heap_header));
    header->size = original_size - (aligned_addr - original_addr);
    header->free = 0;
    header->magic = heap_magic;
    struct heap_footer* footer = (struct heap_footer*) ((uintptr_t) header + sizeof(struct heap_header) + header->size);
    if (footer->magic != heap_magic) {
      panic("Invalid magic value");
    }
    footer->header = header;
    previous_header->size += aligned_addr - original_addr;
    struct heap_footer* new_previous_footer = (struct heap_footer*) ((uintptr_t) header - sizeof(struct heap_footer));
    new_previous_footer->magic = heap_magic;
    new_previous_footer->header = previous_header;
  }
  return header + 1;
}
void free(void* address) {
  struct heap_header* header = address - sizeof(struct heap_header);
  if (header->magic != heap_magic) {
    panic("Invalid magic value");
  }
  header->free = 1;
  struct heap_footer* footer = address + header->size;
  if (footer->magic != heap_magic) {
    panic("Invalid magic value");
  }
  struct heap_footer* previous_footer = (struct heap_footer*) ((uintptr_t) header - sizeof(struct heap_footer));
  if (previous_footer->magic != heap_magic) {
    panic("Invalid magic value");
  }
  struct heap_header* previous_header = previous_footer->header;
  if (previous_header->magic != heap_magic) {
    panic("Invalid magic value");
  }
  if (previous_header->free) {
    remove_sorted_list(&heap_list, &previous_header->list_member);
    previous_header->size += sizeof(struct heap_footer) + sizeof(struct heap_header) + header->size;
    footer->header = previous_header;
    header = previous_header;
  }
  struct heap_header* next_header = (struct heap_header*) (footer + 1);
  asm volatile("nop;nop;nop;nop");
  if ((uintptr_t) next_header < HEAP_START + heap_size) {
    if (next_header->magic != heap_magic) {
      panic("Invalid magic value");
    }
    if (next_header->free) {
      remove_sorted_list(&heap_list, &next_header->list_member);
      header->size += sizeof(struct heap_header) + sizeof(struct heap_footer) + next_header->size;
      footer = (struct heap_footer*) ((uintptr_t) header + sizeof(struct heap_header) + header->size);
      footer->header = header;
    }
  }
  insert_sorted_list(&heap_list, &header->list_member);
}
