#include <panic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

extern void load_gdt(uintptr_t);
extern int boot_stack[];

struct tss {
  uint32_t reserved;
  uint64_t rsp[3];
  uint64_t reserved2;
  uint64_t ist[7];
  uint64_t reserved3;
  uint16_t reserved4;
  uint16_t iopb_offset;
} __attribute__((packed));
struct gdt_entry {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t flags;
  uint8_t base_high;
};
struct gdt_entry_tss {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t flags;
  uint8_t base_high;
  uint32_t base_highest;
  uint32_t reserved;
};
struct gdt_descriptor {
  uint16_t size;
  uint64_t address;
} __attribute__((packed));

struct gdt {
  struct gdt_entry entries[5];
  struct gdt_entry_tss tss;
};

static void encode_entry(size_t i, int access, int flags, struct gdt* gdt) {
  gdt->entries[i].access = access;
  gdt->entries[i].flags = flags;
}
void setup_gdt(bool ap) {
  struct tss* tss = calloc(1, sizeof(struct tss));
  struct gdt* gdt = calloc(1, sizeof(struct gdt));
  struct gdt_descriptor* gdt_descriptor = calloc(1, sizeof(struct gdt_descriptor));
  gdt_descriptor->size = sizeof(struct gdt) - 1;
  gdt_descriptor->address = (uintptr_t) gdt;
  encode_entry(0, 0, 0, gdt);       // Null
  encode_entry(1, 0x9a, 0x20, gdt); // Code
  encode_entry(2, 0x92, 0, gdt);    // Data
  encode_entry(3, 0xf2, 0, gdt);    // User data
  encode_entry(4, 0xfa, 0x20, gdt); // User code
  gdt->tss.base_low = (uintptr_t) tss;
  gdt->tss.base_middle = (uintptr_t) tss >> 16;
  gdt->tss.base_high = (uintptr_t) tss >> 24;
  gdt->tss.base_highest = (uintptr_t) tss >> 32;
  gdt->tss.limit_low = sizeof(struct tss) - 1;
  gdt->tss.access = 0x89;
  gdt->tss.flags = (sizeof(struct tss) - 1) >> 16;
  if (ap) {
    tss->rsp[0] = ((uint64_t*) 0x1000)[2];
  } else {
    tss->rsp[0] = (uintptr_t) boot_stack;
  }
  tss->ist[0] = (uintptr_t) malloc(0x2000) + 0x2000; // NMI
  tss->ist[1] = (uintptr_t) malloc(0x2000) + 0x2000; // Double fault
  tss->iopb_offset = 0xdfff;
  load_gdt((uintptr_t) gdt_descriptor);
  asm volatile("wrmsr"
               :
               : "c"(0xc0000102), "a"(tss), "d"((uintptr_t) tss >> 32));
}
