#include <ctype.h>
#include <stdio.h>
#include <sys/hpet.h>
#include <sys/ipc.h>
#include <sys/scheduler.h>

typedef void (*syscall_handler)(union syscall_args*);

static void syscall_version(union syscall_args* args) {
  if (args->arg0 || args->arg1 || args->arg2 || args->arg3 || args->arg4) {
    puts("Reserved argument is set");
    terminate_current_task(&args->registers);
    return;
  }
  args->return_value = 0;
}
static void syscall_yield(union syscall_args* args) {
  if (args->arg0 || args->arg1 || args->arg2 || args->arg3 || args->arg4) {
    puts("Reserved argument is set");
    terminate_current_task(&args->registers);
    return;
  }
  scheduler(&args->registers);
}
static void syscall_exit(union syscall_args* args) {
  if (args->arg1 || args->arg2 || args->arg3 || args->arg4) {
    puts("Reserved argument is set");
    terminate_current_task(&args->registers);
    return;
  }
  terminate_current_task(&args->registers);
}
static void syscall_get_time(union syscall_args* args) {
  if (args->arg0 || args->arg1 || args->arg2 || args->arg3 || args->arg4) {
    puts("Reserved argument is set");
    terminate_current_task(&args->registers);
    return;
  }
  args->return_value = get_time();
}
static void syscall_reset(union syscall_args* args) {
  if (args->arg0 || args->arg1 || args->arg2 || args->arg3 || args->arg4) {
    puts("Reserved argument is set");
    terminate_current_task(&args->registers);
    return;
  }
  if (!has_process_capability(current_task->process, CAP_ACPI)) {
    puts("No permission to reset system");
    terminate_current_task(&args->registers);
    return;
  }
  uint64_t idt[2] = {0};
  asm volatile("lidt %0; int $0"
               :
               : "m"(idt));
}
static void syscall_log(union syscall_args* args) {
  if (args->arg1 || args->arg2 || args->arg3 || args->arg4) {
    puts("Reserved argument is set");
    terminate_current_task(&args->registers);
    return;
  }
  if (!has_process_capability(current_task->process, CAP_LOG)) {
    puts("No permission to log to console");
    terminate_current_task(&args->registers);
    return;
  }
  if (!isprint(args->arg0) && args->arg0 != '\n') {
    puts("Invalid character");
    terminate_current_task(&args->registers);
    return;
  }
  putchar(args->arg0);
}
static syscall_handler syscall_handlers[256] = {
  syscall_version,
  syscall_yield,
  syscall_exit,
  syscall_spawn_process,
  syscall_start_process,
  syscall_get_pid,
  syscall_get_uid,
  syscall_register_ipc,
  syscall_wait_ipc,
  syscall_return_ipc,
  syscall_set_spawned_uid,
  syscall_grant_ioport,
  syscall_access_ioport,
  syscall_register_irq,
  syscall_clear_irqs,
  syscall_wait_irq,
  syscall_get_ipc_caller_capabilities,
  syscall_grant_capabilities,
  syscall_drop_capabilities,
  syscall_map_phys_memory,
  syscall_get_fb_info,
  syscall_wait,
  syscall_block_ipc_call,
  syscall_unblock_ipc_call,
  syscall_has_arguments,
  syscall_is_caller_child,
  syscall_map_memory,
  syscall_get_time,
  syscall_change_memory_permissions,
  syscall_sleep,
  syscall_change_priority,
  syscall_get_acpi_revision,
  syscall_get_acpi_table,
  syscall_reset,
  syscall_log,
  syscall_listen_exits,
  syscall_get_exited_pid,
  syscall_set_fs,
  syscall_spawn_thread};

void syscall_common(union syscall_args* args) {
  if (args->syscall & 0xffffffffffffff00) {
    syscall_handle_ipc(args);
  } else {
    if (!syscall_handlers[args->syscall]) {
      printf("Invalid syscall %ld\n", args->syscall);
      return terminate_current_task(&args->registers);
    }
    syscall_handlers[args->syscall](args);
  }
}
