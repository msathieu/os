#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <cpu/paging.h>
#include <cpu/smp.h>
#include <elf.h>
#include <heap.h>
#include <panic.h>
#include <string.h>
#include <struct.h>
#include <sys/acpi.h>
#include <sys/lapic.h>
#include <sys/pic.h>
#include <sys/task.h>

struct loader_struct loader_struct;

_Noreturn void kmain(unsigned long magic, uintptr_t structptr) {
  if (magic != 0x375f3b9858ea0482) {
    panic("Magic value is incorrect");
  }
  memcpy(&loader_struct, (void*) structptr, sizeof(struct loader_struct));
  setup_gdt(0);
  setup_idt();
  setup_paging();
  set_cpu_flags();
  setup_heap();
  disable_pic();
  parse_acpi();
  setup_lapic_timer(0);
  start_aps();
  setup_multitasking();
}
