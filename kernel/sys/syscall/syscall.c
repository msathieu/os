#include <ctype.h>
#include <stdio.h>
#include <sys/hpet.h>
#include <sys/ipc.h>
#include <sys/lock.h>
#include <sys/scheduler.h>

typedef void (*syscall_handler)(union syscall_args*);

struct syscall {
  syscall_handler handler;
  bool lock;
  size_t nargs;
};

static void syscall_version(union syscall_args* args) {
  args->return_value = 0;
}
static void syscall_yield(union syscall_args* args) {
  scheduler(&args->registers);
}
static void syscall_exit(union syscall_args* args) {
  if (args->arg1 >= 2) {
    puts("Argument out of range");
    terminate_current_task(&args->registers);
    return;
  }
  if (args->arg1) {
    current_task()->process->should_exit = 1;
  }
  terminate_current_task(&args->registers);
}
static void syscall_get_time(union syscall_args* args) {
  args->return_value = get_time();
}
static void syscall_reset(union syscall_args* args) {
  if (!has_process_capability(current_task()->process, CAP_ACPI)) {
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
  if (!has_process_capability(current_task()->process, CAP_LOG)) {
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
static struct syscall syscall_handlers[256] = {
  {syscall_version, 0, 0},
  {syscall_yield, 1, 0},
  {syscall_exit, 1, 2},
  {syscall_spawn_process, 1, 5},
  {syscall_start_process, 1, 2},
  {syscall_get_pid, 0, 1},
  {syscall_get_uid, 0, 1},
  {syscall_register_ipc, 1, 2},
  {syscall_wait_ipc, 1, 0},
  {syscall_return_ipc, 1, 1},
  {syscall_set_spawned_uid, 1, 1},
  {syscall_grant_ioport, 1, 1},
  {syscall_access_ioport, 0, 4},
  {syscall_register_irq, 1, 2},
  {syscall_clear_irqs, 1, 0},
  {syscall_wait_irq, 1, 0},
  {syscall_get_ipc_caller_capabilities, 1, 1},
  {syscall_grant_capabilities, 1, 2},
  {syscall_drop_capabilities, 1, 2},
  {syscall_map_phys_memory, 1, 3},
  {syscall_get_fb_info, 0, 1},
  {syscall_wait, 1, 0},
  {syscall_block_ipc_call, 1, 0},
  {syscall_unblock_ipc_call, 1, 1},
  {syscall_has_arguments, 0, 1},
  {syscall_is_caller_child, 1, 0},
  {syscall_map_memory, 1, 4},
  {syscall_get_time, 0, 0},
  {syscall_change_memory_permissions, 1, 4},
  {syscall_sleep, 1, 1},
  {syscall_change_priority, 1, 1},
  {syscall_get_acpi_revision, 0, 0},
  {syscall_get_acpi_table, 1, 5},
  {syscall_reset, 1, 0},
  {syscall_log, 1, 1},
  {syscall_listen_exits, 1, 0},
  {syscall_get_exited_pid, 1, 0},
  {syscall_set_fs, 0, 1},
  {syscall_spawn_thread, 1, 3},
  {syscall_fork, 1, 0},
  {syscall_start_fork, 1, 0}};

void syscall_common(union syscall_args* args) {
  if (current_task()->process->should_exit) {
    acquire_lock();
    terminate_current_task(&args->registers);
    return release_lock();
  }
  if (args->syscall & 0xffffffffffffff00) {
    acquire_lock();
    syscall_handle_ipc(args);
    release_lock();
  } else {
    if (!syscall_handlers[args->syscall].handler) {
      acquire_lock();
      printf("Invalid syscall %ld\n", args->syscall);
      terminate_current_task(&args->registers);
      return release_lock();
    }
    if ((syscall_handlers[args->syscall].nargs < 5 && args->arg4) ||
        (syscall_handlers[args->syscall].nargs < 4 && args->arg3) ||
        (syscall_handlers[args->syscall].nargs < 3 && args->arg2) ||
        (syscall_handlers[args->syscall].nargs < 2 && args->arg1) ||
        (syscall_handlers[args->syscall].nargs < 1 && args->arg0)) {
      acquire_lock();
      puts("Reserved argument is set");
      terminate_current_task(&args->registers);
      return release_lock();
    }
    bool lock = syscall_handlers[args->syscall].lock;
    if (lock) {
      acquire_lock();
    }
    syscall_handlers[args->syscall].handler(args);
    if (lock) {
      release_lock();
    }
  }
}
