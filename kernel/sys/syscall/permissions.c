#include <stdio.h>
#include <sys/syscall.h>
#include <sys/task.h>

void syscall_get_uid(union syscall_args* args) {
  if (args->arg1 || args->arg2 || args->arg3 || args->arg4) {
    puts("Reserved argument is set");
    terminate_current_task(&args->registers);
    return;
  }
  switch (args->arg0) {
  case 0:
    args->return_value = current_task()->process->uid;
    break;
  case 1:;
    struct task* requester = current_task()->servicing_syscall_requester;
    if (!requester) {
      puts("Not currently handling IPC call");
      terminate_current_task(&args->registers);
      return;
    }
    args->return_value = requester->process->uid;
    break;
  default:
    puts("Argument out of range");
    terminate_current_task(&args->registers);
  }
}
void syscall_set_spawned_uid(union syscall_args* args) {
  if (args->arg1 || args->arg2 || args->arg3 || args->arg4) {
    puts("Reserved argument is set");
    terminate_current_task(&args->registers);
    return;
  }
  if (!current_task()->spawned_process) {
    puts("No process is currently being spawned");
    terminate_current_task(&args->registers);
    return;
  }
  if (current_task()->process->uid) {
    puts("Only root can change uid");
    terminate_current_task(&args->registers);
    return;
  }
  current_task()->spawned_process->uid = args->arg0;
}
void syscall_grant_capabilities(union syscall_args* args) {
  if (args->arg2 || args->arg3 || args->arg4) {
    puts("Reserved argument is set");
    terminate_current_task(&args->registers);
    return;
  }
  if (args->arg0 >= 64) {
    puts("Capability namespace out of range");
    terminate_current_task(&args->registers);
    return;
  }
  if (!has_process_capability(current_task()->process, CAP_MANAGE) && (args->arg1 & ~current_task()->process->capabilities[args->arg0])) {
    puts("Not allowed to grant capabilities");
    terminate_current_task(&args->registers);
    return;
  }
  if (!current_task()->spawned_process) {
    puts("No process is currently being spawned");
    terminate_current_task(&args->registers);
    return;
  }
  current_task()->spawned_process->capabilities[args->arg0] |= args->arg1;
}
void syscall_drop_capabilities(union syscall_args* args) {
  if (args->arg2 || args->arg3 || args->arg4) {
    puts("Reserved argument is set");
    terminate_current_task(&args->registers);
    return;
  }
  if (args->arg0 >= 64) {
    puts("Capability namespace out of range");
    terminate_current_task(&args->registers);
    return;
  }
  __atomic_and_fetch(&current_task()->process->capabilities[args->arg0], ~args->arg1, __ATOMIC_SEQ_CST);
}
