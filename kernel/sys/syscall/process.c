#include <elf.h>
#include <stdio.h>
#include <string.h>
#include <struct.h>
#include <sys/hpet.h>
#include <sys/lock.h>
#include <sys/scheduler.h>
#include <sys/syscall.h>

extern void jmp_user(uintptr_t, uintptr_t, uintptr_t);

void syscall_spawn_process(union syscall_args* args) {
  if (current_task()->spawned_process) {
    puts("Already spawning process");
    terminate_current_task(&args->registers);
    return;
  }
  char name[6] = {
    args->arg0,
    args->arg1,
    args->arg2,
    args->arg3,
    args->arg4};
  if (!name[0] || strchr(name, '.')) {
    puts("Invalid name");
    terminate_current_task(&args->registers);
    return;
  }
  int file_i = -1;
  for (size_t i = 0; i < 64; i++) {
    if (!strcmp((char*) loader_struct.files[i].name, name)) {
      file_i = i;
    }
  }
  if (file_i == -1) {
    puts("Requested file not found, terminating task");
    terminate_current_task(&args->registers);
    return;
  }
  spawn_child(0);
  current_task()->spawned_process->file_i = file_i;
  args->return_value = current_task()->spawned_process->pid;
}
void syscall_start_process(union syscall_args* args) {
  if (args->arg0 >= 2) {
    puts("Argument out of range");
    terminate_current_task(&args->registers);
    return;
  }
  if (args->arg1 >= 2) {
    puts("Argument out of range");
    terminate_current_task(&args->registers);
    return;
  }
  if (!current_task()->spawned_process) {
    puts("No process ready to start");
    terminate_current_task(&args->registers);
    return;
  }
  current_task()->spawned_process->has_arguments = args->arg0;
  current_task()->spawned_process->has_environment_vars = args->arg1;
  struct task* task = create_task(current_task()->spawned_process);
  current_task()->spawned_process = 0;
  switch_task(task, &args->registers);
  load_elf(current_task()->process->file_i);
}
void syscall_get_pid(union syscall_args* args) {
  switch (args->arg0) {
  case 0:
    args->return_value = current_task()->process->pid;
    break;
  case 1:
    acquire_lock();
    struct task* requester = current_task()->servicing_syscall_requester;
    if (!requester) {
      puts("Not currently handling IPC call");
      terminate_current_task(&args->registers);
      return release_lock();
    }
    args->return_value = requester->process->pid;
    release_lock();
    break;
  case 2:
    acquire_lock();
    if (current_task()->process->parent) {
      args->return_value = current_task()->process->parent->pid;
    } else {
      args->return_value = 0;
    }
    release_lock();
    break;
  case 3:
    acquire_lock();
    if (!current_task()->servicing_syscall_requester) {
      puts("Not currently handling IPC call");
      terminate_current_task(&args->registers);
      return release_lock();
    }
    if (current_task()->servicing_syscall_requester->spawned_process) {
      args->return_value = current_task()->servicing_syscall_requester->spawned_process->pid;
    } else {
      args->return_value = 0;
    }
    release_lock();
    break;
  default:
    acquire_lock();
    puts("Argument out of range");
    terminate_current_task(&args->registers);
    release_lock();
  }
}
void syscall_wait(union syscall_args* args) {
  for (struct process* child = current_task()->process->first_child; child; child = child->next_sibling) {
    if (child->exited) {
      args->return_value = child->pid;
      remove_process(child);
      return;
    }
  }
  if (current_task()->process->waiting_task) {
    puts("Can't wait on multiple threads");
    terminate_current_task(&args->registers);
    return;
  }
  current_task()->process->waiting_task = current_task();
  block_current_task(&args->registers);
}
void syscall_has_arguments(union syscall_args* args) {
  switch (args->arg0) {
  case 0:
    args->return_value = current_task()->process->has_arguments;
    break;
  case 1:
    args->return_value = current_task()->process->has_environment_vars;
    break;
  default:
    acquire_lock();
    puts("Argument out of range");
    terminate_current_task(&args->registers);
    release_lock();
  }
}
void syscall_sleep(union syscall_args* args) {
  sleep_current_task(args->arg0, &args->registers);
}
void syscall_change_priority(union syscall_args* args) {
  if (args->arg0 > PRIORITY_USER_LOW) {
    puts("Argument out of range");
    terminate_current_task(&args->registers);
    return;
  }
  if (!has_process_capability(current_task()->process, CAP_PRIORITY) && args->arg0 <= PRIORITY_SYSTEM_LOW) {
    puts("Not allowed to set this priority");
    terminate_current_task(&args->registers);
    return;
  }
  if (!has_process_capability(current_task()->process, CAP_PRIORITY) && current_task()->process->uid && args->arg0 <= PRIORITY_ROOT_LOW) {
    puts("Not allowed to set this priority");
    terminate_current_task(&args->registers);
    return;
  }
  current_task()->priority = args->arg0;
  scheduler(&args->registers);
}
void syscall_set_fs(union syscall_args* args) {
  if (args->arg0 >= PAGING_USER_PHYS_MAPPINGS_START) {
    acquire_lock();
    puts("Invalid FS value");
    terminate_current_task(&args->registers);
    return release_lock();
  }
  current_task()->fs = args->arg0;
  asm volatile("wrmsr"
               :
               : "c"(0xc0000100), "a"(current_task()->fs), "d"(current_task()->fs >> 32));
}
void syscall_spawn_thread(union syscall_args* args) {
  if (args->arg0 >= PAGING_USER_PHYS_MAPPINGS_START) {
    puts("Invalid thread start");
    terminate_current_task(&args->registers);
    return;
  }
  struct task* task = create_task(current_task()->process);
  uintptr_t arg0 = args->arg0;
  uintptr_t arg1 = args->arg1;
  uintptr_t arg2 = args->arg2;
  switch_task(task, &args->registers);
  release_lock();
  jmp_user(arg1, arg2, arg0);
}
void syscall_fork(union syscall_args* args) {
  if (current_task()->spawned_process) {
    puts("Already spawning process");
    terminate_current_task(&args->registers);
    return;
  }
  spawn_child(1);
}
void syscall_start_fork(union syscall_args* args) {
  if (!current_task()->spawned_process) {
    puts("No process ready to start");
    terminate_current_task(&args->registers);
    return;
  }
  args->return_value = current_task()->spawned_process->pid;
  struct task* task = create_task(current_task()->spawned_process);
  current_task()->spawned_process = 0;
  memcpy(&task->registers, &args->registers, sizeof(struct isr_registers));
  asm volatile("fxsave (%0)"
               :
               : "r"(task->fxsave_region));
  task->registers.rax = 0;
  task->fs = current_task()->fs;
  schedule_task(task, &args->registers);
}
