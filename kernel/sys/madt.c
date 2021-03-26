#include <stddef.h>
#include <sys/acpi.h>
#include <sys/ioapic.h>
#include <sys/lapic.h>

struct acpi_madt {
  struct acpi_header header;
  uint32_t lapic_addr;
  uint32_t flags;
} __attribute__((packed));
struct acpi_madt_entry {
  uint8_t type;
  uint8_t size;
};
#define ACPI_MADT_LAPIC 0
#define ACPI_MADT_IOAPIC 1
#define ACPI_MADT_INTERRUPT_OVERRIDE 2
#define ACPI_MADT_NMI 4
#define ACPI_MADT_LAPIC_ADDRESS_OVERRIDE 5
struct acpi_madt_lapic_address_override {
  uint8_t type;
  uint8_t size;
  uint16_t reserved;
  uint64_t address;
} __attribute__((packed));

uintptr_t madt_lapic_address;
struct acpi_madt_lapic* madt_lapics[256];
size_t madt_num_lapics;
struct acpi_madt_ioapic* madt_ioapics[256];
size_t madt_num_ioapics;
struct acpi_madt_interrupt_override* madt_overrides[16];
size_t madt_num_overrides;
struct acpi_madt_nmi* madt_nmis[256];
size_t madt_num_nmis;

void parse_madt(struct acpi_header* header) {
  struct acpi_madt* madt = (struct acpi_madt*) header;
  madt_lapic_address = madt->lapic_addr;
  for (
    struct acpi_madt_entry* entry = (struct acpi_madt_entry*) (madt + 1);
    (uintptr_t) entry < (uintptr_t) madt + madt->header.size;
    entry = (struct acpi_madt_entry*) ((uintptr_t) entry + entry->size)) {
    switch (entry->type) {
    case ACPI_MADT_LAPIC:
      madt_lapics[madt_num_lapics++] = (struct acpi_madt_lapic*) entry;
      break;
    case ACPI_MADT_IOAPIC:
      madt_ioapics[madt_num_ioapics++] = (struct acpi_madt_ioapic*) entry;
      break;
    case ACPI_MADT_INTERRUPT_OVERRIDE:
      madt_overrides[madt_num_overrides++] = (struct acpi_madt_interrupt_override*) entry;
      break;
    case ACPI_MADT_NMI:
      madt_nmis[madt_num_nmis++] = (struct acpi_madt_nmi*) entry;
      break;
    case ACPI_MADT_LAPIC_ADDRESS_OVERRIDE:;
      struct acpi_madt_lapic_address_override* address_override = (struct acpi_madt_lapic_address_override*) entry;
      madt_lapic_address = address_override->address;
      break;
    }
  }
  setup_lapic(0);
  setup_ioapics();
}
