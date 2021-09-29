#include <cpu/ioports.h>
#include <cpu/paging.h>
#include <stdio.h>
#include <string.h>
#include <struct.h>
#include <sys/acpi.h>
#include <sys/ioapic.h>
#include <sys/lock.h>
#include <sys/scheduler.h>
#include <sys/syscall.h>

struct process* ioports_process[0x10000];
struct process* isa_irqs_process[16];
static bool isa_irqs_fired[16];
struct process* sci_process;
static bool sci_fired;

void syscall_grant_ioport(union syscall_args* args) {
  if (!has_process_capability(current_task()->process, CAP_IOPORT)) {
    puts("Not allowed to register port access");
    terminate_current_task(&args->registers);
    return;
  }
  if (!current_task()->spawned_process) {
    puts("No process is currently being spawned");
    terminate_current_task(&args->registers);
    return;
  }
  if (args->arg0 >= 0x10000) {
    puts("Invalid port");
    terminate_current_task(&args->registers);
    return;
  }
  if (ioports_process[args->arg0]) {
    puts("Requested port has already been granted");
    terminate_current_task(&args->registers);
    return;
  }
  __atomic_store_n(&ioports_process[args->arg0], current_task()->spawned_process, __ATOMIC_SEQ_CST);
  current_task()->spawned_process->ioports_assigned = 1;
}
void syscall_access_ioport(union syscall_args* args) {
  if (args->arg0 >= 0x10000) {
    acquire_lock();
    puts("Invalid port");
    terminate_current_task(&args->registers);
    return release_lock();
  }
  if (__atomic_load_n(&ioports_process[args->arg0], __ATOMIC_SEQ_CST) != current_task()->process && !has_process_capability(current_task()->process, CAP_IOPORT)) {
    acquire_lock();
    puts("No permission to access port");
    terminate_current_task(&args->registers);
    return release_lock();
  }
  switch (args->arg1) {
  case 0:
    switch (args->arg2) {
    case 0:
      args->return_value = inb(args->arg0);
      break;
    case 1:
      args->return_value = inw(args->arg0);
      break;
    case 2:
      args->return_value = inl(args->arg0);
      break;
    default:
      acquire_lock();
      puts("Argument out of range");
      terminate_current_task(&args->registers);
      release_lock();
    }
    break;
  case 1:
    switch (args->arg2) {
    case 0:
      outb(args->arg0, args->arg3);
      break;
    case 1:
      outw(args->arg0, args->arg3);
      break;
    case 2:
      outl(args->arg0, args->arg3);
      break;
    default:
      acquire_lock();
      puts("Argument out of range");
      terminate_current_task(&args->registers);
      release_lock();
    }
    break;
  default:
    acquire_lock();
    puts("Argument out of range");
    terminate_current_task(&args->registers);
    release_lock();
  }
}
static void usermode_irq_handler(struct isr_registers* registers) {
  size_t isr = registers->isr;
  if (isr == 253) {
    struct task* handler = sci_process->irq_handler;
    if (handler) {
      handler->blocked = 0;
      sci_process->irq_handler = 0;
      schedule_task(handler, registers);
    } else {
      sci_fired = 1;
    }
  } else {
    struct task* handler = isa_irqs_process[isr - 48]->irq_handler;
    if (handler) {
      handler->blocked = 0;
      handler->registers.rax = isr - 48;
      isa_irqs_process[isr - 48]->irq_handler = 0;
      schedule_task(handler, registers);
    } else {
      isa_irqs_fired[isr - 48] = 1;
    }
  }
}
void syscall_register_irq(union syscall_args* args) {
  if (!has_process_capability(current_task()->process, CAP_IRQ)) {
    puts("Not allowed to register IRQ");
    terminate_current_task(&args->registers);
    return;
  }
  if (!current_task()->spawned_process) {
    puts("No process is currently being spawned");
    terminate_current_task(&args->registers);
    return;
  }
  switch (args->arg0) {
  case 0:
    if (args->arg1 >= 16) {
      puts("Invalid IRQ");
      terminate_current_task(&args->registers);
      return;
    }
    if (isr_handlers[48 + args->arg1]) {
      puts("Requested IRQ has already been registered");
      terminate_current_task(&args->registers);
      return;
    }
    isa_irqs_process[args->arg1] = current_task()->spawned_process;
    register_isa_irq(args->arg1, usermode_irq_handler);
    current_task()->spawned_process->irqs_assigned = 1;
    break;
  case 1:
    if (args->arg1 != 253) {
      puts("Invalid IRQ");
      terminate_current_task(&args->registers);
      return;
    }
    if (!has_process_capability(current_task()->process, CAP_ACPI)) {
      puts("Not allowed to register IRQ");
      terminate_current_task(&args->registers);
      return;
    }
    if (isr_handlers[253]) {
      puts("Requested IRQ has already been registered");
      terminate_current_task(&args->registers);
      return;
    }
    sci_process = current_task()->spawned_process;
    isr_handlers[253] = usermode_irq_handler;
    current_task()->spawned_process->irqs_assigned = 1;
    break;
  default:
    puts("Argument out of range");
    terminate_current_task(&args->registers);
    return;
  }
}
void syscall_clear_irqs(union syscall_args* args) {
  if (!current_task()->process->irqs_assigned) {
    puts("No permission to handle IRQs");
    terminate_current_task(&args->registers);
    return;
  }
  for (size_t i = 0; i < 16; i++) {
    if (isa_irqs_process[i] == current_task()->process) {
      isa_irqs_fired[i] = 0;
    }
  }
  if (sci_process == current_task()->process) {
    sci_fired = 0;
  }
}
void syscall_wait_irq(union syscall_args* args) {
  if (!current_task()->process->irqs_assigned) {
    puts("No permission to handle IRQs");
    terminate_current_task(&args->registers);
    return;
  }
  if (current_task()->process->irq_handler) {
    puts("Already waiting for IRQ");
    terminate_current_task(&args->registers);
    return;
  }
  for (size_t i = 0; i < 16; i++) {
    if (isa_irqs_process[i] == current_task()->process && isa_irqs_fired[i]) {
      isa_irqs_fired[i] = 0;
      args->return_value = i;
      return;
    }
  }
  if (sci_process == current_task()->process && sci_fired) {
    sci_fired = 0;
    args->return_value = 253;
    return;
  }
  current_task()->process->irq_handler = current_task();
  block_current_task(&args->registers);
}
void syscall_map_phys_memory(union syscall_args* args) {
  if (!has_process_capability(current_task()->process, CAP_MAP_MEMORY)) {
    puts("No permission to map memory");
    terminate_current_task(&args->registers);
    return;
  }
  if (args->arg0 % 0x1000 || args->arg1 % 0x1000 || !args->arg1) {
    puts("Invalid address or size");
    terminate_current_task(&args->registers);
    return;
  }
  if (args->arg2 >= 2) {
    puts("Argument out of range");
    terminate_current_task(&args->registers);
    return;
  }
  if (args->arg2 && !current_task()->spawned_process) {
    puts("No process is currently being spawned");
    terminate_current_task(&args->registers);
    return;
  }
  if (args->arg2) {
    physical_mappings_process = current_task()->spawned_process;
  } else {
    physical_mappings_process = current_task()->process;
  }
  args->return_value = get_free_ipc_range(args->arg1);
  physical_mappings_process = 0;
  if (!args->return_value) {
    puts("Out of memory reserved for physical mappings");
    terminate_current_task(&args->registers);
    return;
  }
  if (args->arg2) {
    switch_pml4(current_task()->spawned_process->address_space);
  }
  for (size_t i = 0; i < args->arg1; i += 0x1000) {
    create_mapping(args->return_value + i, args->arg0 + i, 1, 1, 0, 1);
  }
  if (args->arg2) {
    switch_pml4(current_task()->process->address_space);
  }
}
void syscall_get_fb_info(union syscall_args* args) {
  if (!has_process_capability(current_task()->process, CAP_GET_FB_INFO)) {
    acquire_lock();
    puts("No permission to get framebuffer info");
    terminate_current_task(&args->registers);
    return release_lock();
  }
  switch (args->arg0) {
  case 0:
    args->return_value = loader_struct.fb_address;
    break;
  case 1:
    args->return_value = loader_struct.fb_width;
    break;
  case 2:
    args->return_value = loader_struct.fb_height;
    break;
  case 3:
    args->return_value = loader_struct.fb_pitch;
    break;
  case 4:
    args->return_value = loader_struct.fb_bpp;
    break;
  case 5:
    args->return_value = loader_struct.fb_red_index;
    break;
  case 6:
    args->return_value = loader_struct.fb_green_index;
    break;
  case 7:
    args->return_value = loader_struct.fb_blue_index;
    break;
  default:
    acquire_lock();
    puts("Argument out of range");
    terminate_current_task(&args->registers);
    release_lock();
  }
}
void syscall_get_acpi_revision(union syscall_args* args) {
  if (!has_process_capability(current_task()->process, CAP_ACPI)) {
    acquire_lock();
    puts("No permission to get ACPI revision");
    terminate_current_task(&args->registers);
    return release_lock();
  }
  args->return_value = acpi_revision;
}
void syscall_get_acpi_table(union syscall_args* args) {
  if (!has_process_capability(current_task()->process, CAP_ACPI)) {
    puts("No permission to get ACPI table");
    terminate_current_task(&args->registers);
    return;
  }
  char name[4] = {
    args->arg0,
    args->arg1,
    args->arg2,
    args->arg3};
  struct acpi_header* table = acpi_find_table(name, 0, args->arg4);
  if (!table) {
    args->return_value = 0;
    return;
  }
  args->return_value = get_free_range(table->size, 0, 1, 0, 0x1000);
  if (!args->return_value) {
    puts("Not enough space for requested table");
    terminate_current_task(&args->registers);
    return;
  }
  memcpy((void*) args->return_value, table, table->size);
  set_paging_flags(args->return_value, table->size, 1, 0, 0);
}
