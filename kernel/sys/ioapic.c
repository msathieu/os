#include <cpu/isr.h>
#include <cpu/paging.h>
#include <panic.h>
#include <sys/lapic.h>
#include <sys/madt.h>
#define IOAPIC_REGISTER_VERSION 1
#define IOAPIC_REGISTER_REDIRECTIONS 0x10

struct acpi_madt_ioapic {
  uint8_t type;
  uint8_t size;
  uint8_t ioapic_id;
  uint8_t reserved;
  uint32_t address;
  uint32_t gsi_base;
} __attribute__((packed));
struct acpi_madt_interrupt_override {
  uint8_t type;
  uint8_t size;
  uint8_t bus;
  uint8_t source;
  uint32_t gsi;
  uint16_t flags;
} __attribute__((packed));
static struct ioapic {
  int gsi_start;
  int gsi_end;
} ioapics[256];
union ioapic_redirection {
  struct {
    uint64_t vector : 8;
    uint64_t delivery_mode : 3;
    uint64_t destination_mode : 1;
    uint64_t delivery_status : 1;
    uint64_t polarity : 1;
    uint64_t remote_irr : 1;
    uint64_t trigger_mode : 1;
    uint64_t masked : 1;
    uint64_t destination : 8;
  };
  struct {
    uint32_t lower;
    uint32_t higher;
  };
};

static volatile uint32_t* registers[256];

static uint32_t read_register(size_t ioapic_i, size_t register_i) {
  registers[ioapic_i][0] = register_i;
  return registers[ioapic_i][4];
}
static void write_register(size_t ioapic_i, size_t register_i, uint32_t value) {
  registers[ioapic_i][0] = register_i;
  registers[ioapic_i][4] = value;
}
static size_t get_ioapic_id(int gsi) {
  for (size_t i = 0; i < madt_num_ioapics; i++) {
    if (ioapics[i].gsi_start <= gsi && gsi <= ioapics[i].gsi_end) {
      return i;
    }
  }
  panic("Couldn't find I/O APIC for specified GSI");
}
static void write_redirection(int gsi, union ioapic_redirection redirection) {
  size_t ioapic_i = get_ioapic_id(gsi);
  gsi -= ioapics[ioapic_i].gsi_start;
  write_register(ioapic_i, IOAPIC_REGISTER_REDIRECTIONS + gsi * 2, redirection.lower);
  write_register(ioapic_i, IOAPIC_REGISTER_REDIRECTIONS + gsi * 2 + 1, redirection.higher);
}
static void mask_gsi(int gsi) {
  union ioapic_redirection redirection = {.masked = 1};
  write_redirection(gsi, redirection);
}
static void map_gsi(int gsi, int vector, bool polarity, bool trigger_mode) {
  union ioapic_redirection redirection = {.vector = vector, .polarity = polarity, .trigger_mode = trigger_mode, .destination = madt_bsp_lapic_id};
  write_redirection(gsi, redirection);
}
void register_isa_irq(int irq, isr_handler handler) {
  if (isr_handlers[48 + irq]) {
    panic("Interrupt handler already registered");
  }
  isr_handlers[48 + irq] = handler;
  int gsi = irq;
  bool polarity = 0;
  bool trigger_mode = 0;
  for (size_t i = 0; i < madt_num_overrides; i++) {
    struct acpi_madt_interrupt_override* override = madt_overrides[i];
    if (override->source == irq) {
      gsi = override->gsi;
      polarity = override->flags & 3;
      trigger_mode = override->flags & 0xc;
      break;
    }
  }
  map_gsi(gsi, 48 + irq, polarity, trigger_mode);
}
void unregister_isa_irq(int irq) {
  isr_handlers[48 + irq] = 0;
  int gsi = irq;
  for (size_t i = 0; i < madt_num_overrides; i++) {
    struct acpi_madt_interrupt_override* override = madt_overrides[i];
    if (override->source == irq) {
      gsi = override->gsi;
      break;
    }
  }
  mask_gsi(gsi);
}
void setup_ioapics(void) {
  for (size_t i = 0; i < madt_num_ioapics; i++) {
    registers[i] = map_physical(madt_ioapics[i]->address, 0x14, 1, 1);
    ioapics[i].gsi_start = madt_ioapics[i]->gsi_base;
    ioapics[i].gsi_end = ioapics[i].gsi_start + (uint8_t) (read_register(i, IOAPIC_REGISTER_VERSION) >> 16);
    for (int j = ioapics[i].gsi_start; j <= ioapics[i].gsi_end; j++) {
      mask_gsi(j);
    }
  }
  asm volatile("sti");
}
