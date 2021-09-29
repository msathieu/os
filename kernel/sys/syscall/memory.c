#include <cpu/paging.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <sys/task.h>

void syscall_map_memory(union syscall_args* args) {
  if (args->arg1 >= 2) {
    puts("Argument out of range");
    terminate_current_task(&args->registers);
    return;
  }
  if (args->arg2 >= 2) {
    puts("Argument out of range");
    terminate_current_task(&args->registers);
    return;
  }
  if (args->arg1 && args->arg2) {
    puts("Memory can't be both writable and executable");
    terminate_current_task(&args->registers);
    return;
  }
  if (args->arg3 % 0x1000 || !args->arg3 || args->arg3 >= PAGING_USER_PHYS_MAPPINGS_START) {
    puts("Invalid starting position");
    terminate_current_task(&args->registers);
    return;
  }
  uintptr_t address = get_free_range(args->arg0, 1, args->arg1, args->arg2, args->arg3);
  if (!address) {
    puts("Not enough space for requested mapping");
    terminate_current_task(&args->registers);
    return;
  }
  args->return_value = address;
}
void syscall_change_memory_permissions(union syscall_args* args) {
  if (args->arg2 >= 2) {
    puts("Argument out of range");
    terminate_current_task(&args->registers);
    return;
  }
  if (args->arg3 >= 2) {
    puts("Argument out of range");
    terminate_current_task(&args->registers);
    return;
  }
  if (args->arg2 && args->arg3) {
    puts("Memory can't be both writable and executable");
    terminate_current_task(&args->registers);
    return;
  }
  if (args->arg0 % 0x1000 || args->arg1 % 0x1000) {
    puts("Invalid position or size");
    terminate_current_task(&args->registers);
    return;
  }
  uintptr_t end;
  if (__builtin_uaddl_overflow(args->arg0, args->arg1, &end)) {
    puts("Invalid position or size");
    terminate_current_task(&args->registers);
    return;
  }
  if (end >= PAGING_USER_PHYS_MAPPINGS_START) {
    puts("Invalid position or size");
    terminate_current_task(&args->registers);
    return;
  }
  for (size_t i = args->arg0; i < end; i += 0x1000) {
    if (!is_page_mapped(i, 0)) {
      puts("Memory region doesn't exist");
      terminate_current_task(&args->registers);
      return;
    }
  }
  set_paging_flags(args->arg0, args->arg1, 1, args->arg2, args->arg3);
}
