#include <stdio.h>
#include <sys/lock.h>
#include <sys/syscall.h>
#include <sys/task.h>

void syscall_get_uid(union syscall_args* args) {
  switch (args->arg0) {
  case 0:
    args->return_value = __atomic_load_n(&current_task()->process->uid, __ATOMIC_SEQ_CST);
    break;
  case 1:
    acquire_lock();
    struct task* requester = current_task()->servicing_syscall_requester;
    if (!requester) {
      puts("Not currently handling IPC call");
      terminate_current_task(&args->registers);
      return release_lock();
    }
    args->return_value = requester->process->uid;
    release_lock();
    break;
  default:
    acquire_lock();
    puts("Argument out of range");
    terminate_current_task(&args->registers);
    release_lock();
  }
}
void syscall_set_spawned_uid(union syscall_args* args) {
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
  if (args->arg0 >= 64) {
    puts("Capability namespace out of range");
    terminate_current_task(&args->registers);
    return;
  }
  __atomic_and_fetch(&current_task()->process->capabilities[args->arg0], ~args->arg1, __ATOMIC_SEQ_CST);
}
