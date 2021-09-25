#include <stdio.h>
#include <sys/ipc.h>
#include <sys/task.h>

void syscall_register_ipc(union syscall_args* args) {
  if (args->arg2 || args->arg3 || args->arg4) {
    puts("Reserved argument is set");
    terminate_current_task(&args->registers);
    return;
  }
  if (args->arg0 > 1) {
    puts("Argument is out of range");
    terminate_current_task(&args->registers);
    return;
  }
  if (args->arg1 > 1) {
    puts("Argument is out of range");
    terminate_current_task(&args->registers);
    return;
  }
  if (current_task()->process->accepts_syscalls == (int) args->arg0) {
    puts("Desired registration status is already set");
    terminate_current_task(&args->registers);
    return;
  }
  current_task()->process->accepts_syscalls = args->arg0;
  if (args->arg0) {
    insert_linked_list(&syscall_processes, &current_task()->process->list_member);
    current_task()->process->accepts_shared_memory = args->arg1;
  } else {
    remove_linked_list(&syscall_processes, &current_task()->process->list_member);
  }
}
void syscall_get_ipc_caller_capabilities(union syscall_args* args) {
  if (args->arg1 || args->arg2 || args->arg3 || args->arg4) {
    puts("Reserved argument is set");
    terminate_current_task(&args->registers);
    return;
  }
  if (!args->arg0) {
    puts("Not allowed to access kernel capabilities");
    terminate_current_task(&args->registers);
    return;
  }
  if (args->arg0 >= 64) {
    puts("Capability namespace out of range");
    terminate_current_task(&args->registers);
    return;
  }
  struct task* requester = current_task()->servicing_syscall_requester;
  if (!requester) {
    puts("Not currently handling IPC call");
    terminate_current_task(&args->registers);
    return;
  }
  if (has_process_capability(requester->process, CAP_MANAGE)) {
    args->return_value = 0xffffffffffffffff;
  } else {
    args->return_value = requester->process->capabilities[args->arg0];
  }
}
static bool has_child_with_pid(size_t pid, struct process* process) {
  for (struct process* child = process->first_child; child; child = child->next_sibling) {
    if (!child->exited) {
      if (child->pid == pid || has_child_with_pid(pid, child)) {
        return 1;
      }
    }
  }
  return 0;
}
void syscall_is_caller_child(union syscall_args* args) {
  if (args->arg0 || args->arg1 || args->arg2 || args->arg3 || args->arg4) {
    puts("Reserved argument is set");
    terminate_current_task(&args->registers);
    return;
  }
  if (!current_task()->servicing_syscall_requester) {
    puts("Not currently handling IPC call");
    terminate_current_task(&args->registers);
    return;
  }
  args->return_value = has_child_with_pid(current_task()->servicing_syscall_requester->process->pid, current_task()->process);
}
