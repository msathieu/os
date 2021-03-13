#include <stddef.h>
#include <stdint.h>

struct gdt_entry {
  uint16_t limit_low;
  uint16_t base_low;
  uint8_t base_middle;
  uint8_t access;
  uint8_t flags;
  uint8_t base_high;
};
struct gdt_descriptor {
  uint16_t size;
  uint64_t address;
} __attribute__((packed)) gdt_descriptor;

static struct gdt_entry gdt[3];

static void encode_entry(size_t i, size_t limit, int access, int flags) {
  gdt[i].limit_low = limit;
  gdt[i].access = access;
  gdt[i].flags = limit >> 16;
  gdt[i].flags |= flags;
}
void setup_gdt(void) {
  gdt_descriptor.size = sizeof(struct gdt_entry) * 3 - 1;
  gdt_descriptor.address = (uintptr_t) gdt;
  encode_entry(0, 0, 0, 0);       // Null
  encode_entry(1, 0, 0x9a, 0x20); // Code
  encode_entry(2, 0, 0x92, 0);    // Data
}
