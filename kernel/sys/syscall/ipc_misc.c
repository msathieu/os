#include <stdio.h>
#include <sys/ipc.h>
#include <sys/process.h>
#include <sys/task.h>

static void register_ipc(struct process* process) {
  if (!current_task()->process->accepts_syscalls) {
    linked_list_insert(&ipc_handling_processes, &process->ipc_list_member, process);
  }
  process->accepts_syscalls = true;
}
void syscall_register_ipc(__attribute__((unused)) union syscall_args* args) {
  register_ipc(current_task()->process);
}
void syscall_get_ipc_caller_capabilities(union syscall_args* args) {
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
  for (struct linked_list_member* member = process->children_list.first; member; member = member->next) {
    struct process* child = member->node;
    if (!child->exited) {
      if (child->pid == pid || has_child_with_pid(pid, child)) {
        return 1;
      }
    }
  }
  return 0;
}
void syscall_is_caller_child(union syscall_args* args) {
  if (!current_task()->servicing_syscall_requester) {
    puts("Not currently handling IPC call");
    terminate_current_task(&args->registers);
    return;
  }
  args->return_value = has_child_with_pid(current_task()->servicing_syscall_requester->process->pid, current_task()->process);
}
void syscall_register_ipc_name(union syscall_args* args) {
  if (!has_process_capability(current_task()->process, CAP_REGISTER_IPC_NAME)) {
    puts("Not allowed to register IPC name");
    terminate_current_task(&args->registers);
    return;
  }
  if (!current_task()->spawned_process) {
    puts("Not spawning process");
    terminate_current_task(&args->registers);
    return;
  }
  struct process* process = current_task()->spawned_process;
  process->ipc_name[0] = args->arg0;
  process->ipc_name[1] = args->arg1;
  process->ipc_name[2] = args->arg2;
  process->ipc_name[3] = args->arg3;
  process->ipc_name[4] = args->arg4;
  register_ipc(process);
}
void syscall_get_ipc_pid(union syscall_args* args) {
  struct process* ipc_process = 0;
  for (struct linked_list_member* member = ipc_handling_processes.first; member; member = member->next) {
    struct process* process = member->node;
    if (process->ipc_name[0] == (char) args->arg0 && process->ipc_name[1] == (char) args->arg1 && process->ipc_name[2] == (char) args->arg2 && process->ipc_name[3] == (char) args->arg3 && process->ipc_name[4] == (char) args->arg4) {
      ipc_process = process;
      break;
    }
  }
  if (!ipc_process) {
    args->return_value = -1;
  } else {
    args->return_value = ipc_process->pid;
  }
}
