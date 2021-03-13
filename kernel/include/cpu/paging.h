#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define PAGING_PHYSICAL_MAPPINGS_START 0xffff800000000000
#define PAGING_USER_PHYS_MAPPINGS_START 0x7fffc0000000
#define PAGING_PHYSICAL_MAPPINGS_SIZE 0x40000000

struct paging_table;

uintptr_t convert_to_physical(uintptr_t, struct paging_table*);
void create_mapping(uintptr_t, uintptr_t, bool, bool, bool, bool);
struct paging_table* create_pml4(void);
void destroy_pml4(struct paging_table*);
void free_page(uintptr_t);
void unmap_page(uintptr_t);
uintptr_t get_free_range(size_t, bool, bool, uintptr_t);
bool is_page_mapped(uintptr_t, bool);
void* map_physical(uintptr_t, size_t, bool, bool, bool);
void map_range(uintptr_t, size_t, bool, bool, bool);
void setup_paging(void);
void set_paging_flags(uintptr_t, size_t, bool, bool, bool);
void switch_pml4(struct paging_table*);

extern uintptr_t user_physical_mappings_addr;
extern struct paging_table* current_pml4;
