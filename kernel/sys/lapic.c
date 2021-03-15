#include <cpu/paging.h>
#include <cpu/smp.h>
#include <stdlib.h>
#include <string.h>
#include <sys/hpet.h>
#include <sys/madt.h>
#include <sys/scheduler.h>
#define LAPIC_REGISTER_EOI 0xb
#define LAPIC_REGISTER_SPURIOUS_INT 0xf
#define LAPIC_REGISTER_INTERRUPT_COMMAND 0x30
#define LAPIC_REGISTER_TIMER 0x32
#define LAPIC_REGISTER_LINT0 0x35
#define LAPIC_REGISTER_LINT1 0x36
#define LAPIC_REGISTER_TIMER_INTIAL_COUNT 0x38
#define LAPIC_REGISTER_TIMER_CURRENT_COUNT 0x39
#define LAPIC_REGISTER_TIMER_DIVIDE 0x3e
#define LAPIC_LVT_DELIVERY_NMI 0x400
#define LAPIC_LVT_POLARITY_LOW 0x2000
#define LAPIC_DELIVERY_INIT 0x500
#define LAPIC_DELIVERY_STARTUP 0x600

struct acpi_madt_lapic {
  uint8_t type;
  uint8_t size;
  uint8_t processor_id;
  uint8_t lapic_id;
  uint32_t flags;
} __attribute__((packed));
struct acpi_madt_nmi {
  uint8_t type;
  uint8_t size;
  uint8_t processor_id;
  uint16_t flags;
  uint8_t lintn;
} __attribute__((packed));

static volatile uint32_t* registers;
static size_t ticks_per_10ms;
size_t madt_bsp_lapic_id;

extern void ap_trampoline(void);

static uint32_t read_register(size_t register_i) {
  return registers[register_i * 4];
}
static void write_register(size_t register_i, uint32_t value) {
  registers[register_i * 4] = value;
}
void lapic_eoi(void) {
  write_register(LAPIC_REGISTER_EOI, 0);
}
void setup_lapic(size_t nlapic) {
  if (!nlapic) {
    registers = map_physical(madt_lapic_address, 0x400, 0, 1, 1);
    madt_bsp_lapic_id = madt_lapics[0]->lapic_id;
  }
  write_register(LAPIC_REGISTER_SPURIOUS_INT, 0x1ff);
  for (size_t i = 0; i < madt_num_nmis; i++) {
    struct acpi_madt_nmi* nmi = madt_nmis[i];
    if (nmi->processor_id == madt_lapics[nlapic]->processor_id || nmi->processor_id == 0xff) {
      uint32_t lint = LAPIC_LVT_DELIVERY_NMI;
      if (nmi->flags & 3) {
        lint |= LAPIC_LVT_POLARITY_LOW;
      }
      if (!nmi->lintn) {
        write_register(LAPIC_REGISTER_LINT0, lint);
      } else if (nmi->lintn == 1) {
        write_register(LAPIC_REGISTER_LINT1, lint);
      }
    }
  }
}
void set_lapic_timer(size_t milliseconds) {
  write_register(LAPIC_REGISTER_TIMER_INTIAL_COUNT, ticks_per_10ms * milliseconds / 10);
}
void setup_lapic_timer(bool ap) {
  write_register(LAPIC_REGISTER_TIMER_DIVIDE, 3);
  if (!ap) {
    write_register(LAPIC_REGISTER_TIMER_INTIAL_COUNT, 0xffffffff);
    sleep(10 * TIME_MILLISECOND);
    ticks_per_10ms = 0xffffffff - read_register(LAPIC_REGISTER_TIMER_CURRENT_COUNT);
    write_register(LAPIC_REGISTER_TIMER_INTIAL_COUNT, 0);
    isr_handlers[254] = scheduler;
  }
  write_register(LAPIC_REGISTER_TIMER, 254);
}
void start_aps(void) {
  create_mapping(0x1000, 0x1000, 0, 1, 0, 0);
  memcpy((void*) 0x1000, &ap_trampoline, 0x1000);
  ((uint64_t*) 0x1000)[3] = convert_to_physical((uintptr_t) current_pml4, current_pml4);
  for (size_t i = 1; i < madt_num_lapics; i++) {
    ap_nlapic = i;
    ((uint64_t*) 0x1000)[2] = (uintptr_t) malloc(0x2000) + 0x2000;
    write_register(LAPIC_REGISTER_INTERRUPT_COMMAND + 1, madt_lapics[i]->lapic_id << 24);
    uint32_t icr = LAPIC_DELIVERY_INIT;
    write_register(LAPIC_REGISTER_INTERRUPT_COMMAND, icr);
    sleep(10 * TIME_MILLISECOND);
    write_register(LAPIC_REGISTER_INTERRUPT_COMMAND + 1, madt_lapics[i]->lapic_id << 24);
    icr = LAPIC_DELIVERY_STARTUP | 1;
    set_paging_flags(0x1000, 0x1000, 0, 0, 1);
    write_register(LAPIC_REGISTER_INTERRUPT_COMMAND, icr);
    while (!ap_startup) {
      asm volatile("pause");
    }
    ap_startup = 0;
    set_paging_flags(0x1000, 0x1000, 0, 1, 0);
  }
  free_page(0x1000);
}
