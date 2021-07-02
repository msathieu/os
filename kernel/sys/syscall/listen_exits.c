#include <stdio.h>
#include <stdlib.h>
#include <sys/process.h>
#include <sys/syscall.h>
#include <sys/task.h>

struct process* exit_listener_processes[64];

void syscall_listen_exits(union syscall_args* args) {
  if (args->arg0 || args->arg1 || args->arg2 || args->arg3 || args->arg4) {
    puts("Reserved argument is set");
    terminate_current_task(&args->registers);
    return;
  }
  if (!has_process_capability(current_task->process, CAP_LISTEN_EXITS)) {
    puts("Not allowed to listen for process exits");
    terminate_current_task(&args->registers);
    return;
  }
  if (current_task->process->exit_listener) {
    puts("Already listening to process exits");
    terminate_current_task(&args->registers);
    return;
  }
  for (size_t i = 0; i < 64; i++) {
    if (!exit_listener_processes[i]) {
      exit_listener_processes[i] = current_task->process;
      current_task->process->exit_listener = 1;
      return;
    }
  }
  puts("Maximum number of exit listener processes has already registered");
  terminate_current_task(&args->registers);
}
void syscall_get_exited_pid(union syscall_args* args) {
  if (args->arg0 || args->arg1 || args->arg2 || args->arg3 || args->arg4) {
    puts("Reserved argument is set");
    terminate_current_task(&args->registers);
    return;
  }
  if (!current_task->process->exit_listener) {
    puts("Not listening to process exits");
    terminate_current_task(&args->registers);
    return;
  }
  struct exited_pid* exited_pid = (struct exited_pid*) current_task->process->exited_pids_list.first;
  if (exited_pid) {
    args->return_value = exited_pid->pid;
    remove_linked_list(&current_task->process->exited_pids_list, &exited_pid->list_member);
    free(exited_pid);
  } else {
    args->return_value = 0;
  }
}
