#include <cpu/paging.h>
#include <panic.h>
#include <sorted_list.h>
#include <sys/hpet.h>
#include <sys/ioapic.h>
#include <sys/scheduler.h>
#define HPET_REGISTER_CAPABILITIES 0
#define HPET_REGISTER_CONFIG 2
#define HPET_REGISTER_COUNTER 0x1e
#define HPET_REGISTER_TIMERS_CONFIG 0x20
#define HPET_REGISTER_TIMERS_COMPARATOR 0x21
#define HPET_CONFIG_ENABLE 1
#define HPET_CONFIG_LEGACY_REPLACEMENT 2
#define HPET_TIMER_CONFIG_ENABLE 4

struct acpi_hpet {
  struct acpi_header header;
  uint8_t revision_id;
  uint8_t comparator_count : 5;
  uint8_t counter_size : 1;
  uint8_t reserved : 1;
  uint8_t legacy_replacement : 1;
  uint16_t pci_vendor_id;
  struct generic_address_structure address;
} __attribute__((packed));

static volatile uint64_t* registers;
static size_t tick_length;
static volatile bool sleep_done;
static bool kernel_sleep;

static int sleep_compare(struct task* task1, struct task* task2) {
  if (task1->sleep_until <= task2->sleep_until) {
    return -1;
  } else {
    return 1;
  }
}

static struct sorted_list sleeping_list = {.compare = (sorted_list_compare) sleep_compare};

static uint64_t read_register(size_t register_i) {
  return registers[register_i];
}
static void write_register(size_t register_i, uint64_t value) {
  registers[register_i] = value;
}
size_t get_time(void) {
  return read_register(HPET_REGISTER_COUNTER) / (TIME_MILLISECOND / tick_length);
}
static void handler(struct isr_registers* isr_registers) {
  if (kernel_sleep) {
    sleep_done = 1;
    kernel_sleep = 0;
    write_register(HPET_REGISTER_TIMERS_CONFIG, read_register(HPET_REGISTER_TIMERS_CONFIG) & ~HPET_TIMER_CONFIG_ENABLE);
  } else {
    struct task* next_task;
    size_t current_time = get_time();
    for (struct task* task = (struct task*) sleeping_list.first; task && current_time >= task->sleep_until; task = next_task) {
      next_task = (struct task*) task->list_member.next;
      remove_sorted_list(&sleeping_list, &task->list_member);
      task->blocked = 0;
      schedule_task(task, isr_registers);
    }
    if (sleeping_list.first) {
      struct task* task = (struct task*) sleeping_list.first;
      size_t duration = task->sleep_until - current_time;
      if (duration < 50) {
        duration = 50;
      }
      size_t comparator = read_register(HPET_REGISTER_COUNTER) + duration * (TIME_MILLISECOND / tick_length);
      write_register(HPET_REGISTER_TIMERS_COMPARATOR, comparator);
    } else {
      write_register(HPET_REGISTER_TIMERS_CONFIG, read_register(HPET_REGISTER_TIMERS_CONFIG) & ~HPET_TIMER_CONFIG_ENABLE);
    }
  }
}
void sleep(size_t time) {
  kernel_sleep = 1;
  size_t comparator = read_register(HPET_REGISTER_COUNTER) + time / tick_length;
  write_register(HPET_REGISTER_TIMERS_COMPARATOR, comparator);
  write_register(HPET_REGISTER_TIMERS_CONFIG, read_register(HPET_REGISTER_TIMERS_CONFIG) | HPET_TIMER_CONFIG_ENABLE);
  asm volatile("cli");
  while (!sleep_done) {
    asm volatile("sti");
    asm volatile("hlt");
    asm volatile("cli");
  }
  asm volatile("sti");
  sleep_done = 0;
}
void sleep_current_task(size_t duration, struct isr_registers* isr_registers) {
  if (duration > 24 * 60 * 60 * 1000) {
    duration = 24 * 60 * 60 * 1000;
  }
  struct task* task = current_task;
  block_current_task(isr_registers);
  task->sleep_until = get_time() + duration;
  insert_sorted_list(&sleeping_list, &task->list_member);
  if ((struct task*) sleeping_list.first == task) {
    if (duration < 50) {
      duration = 50;
    }
    size_t comparator = read_register(HPET_REGISTER_COUNTER) + duration * (TIME_MILLISECOND / tick_length);
    write_register(HPET_REGISTER_TIMERS_COMPARATOR, comparator);
    write_register(HPET_REGISTER_TIMERS_CONFIG, read_register(HPET_REGISTER_TIMERS_CONFIG) | HPET_TIMER_CONFIG_ENABLE);
  }
}
void setup_hpet(struct acpi_header* header) {
  struct acpi_hpet* hpet = (struct acpi_hpet*) header;
  if (!hpet->counter_size) {
    panic("HPET counter isn't 64-bit");
  }
  if (!hpet->legacy_replacement) {
    panic("HPET doesn't support legacy replacement mapping");
  }
  if (hpet->address.address_space != GAS_ADDRESS_MEMORY) {
    panic("HPET isn't memory mapped");
  }
  registers = map_physical(hpet->address.address, 0x400, 0, 1, 1);
  if (!(read_register(HPET_REGISTER_TIMERS_CONFIG) & 0x20)) {
    panic("First HPET timer isn't 64-bit");
  }
  tick_length = read_register(HPET_REGISTER_CAPABILITIES) >> 32;
  write_register(HPET_REGISTER_CONFIG, read_register(HPET_REGISTER_CONFIG) | HPET_CONFIG_LEGACY_REPLACEMENT | HPET_CONFIG_ENABLE);
  register_isa_irq(0, handler);
}
