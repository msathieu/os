#include <stdio.h>
#include <stdlib.h>
#include <sys/process.h>
#include <sys/syscall.h>
#include <sys/task.h>

struct linked_list exit_listener_processes;

void syscall_listen_exits(union syscall_args* args) {
  if (!has_process_capability(current_task()->process, CAP_LISTEN_EXITS)) {
    puts("Not allowed to listen for process exits");
    terminate_current_task(&args->registers);
    return;
  }
  if (current_task()->process->exit_listener) {
    puts("Already listening to process exits");
    terminate_current_task(&args->registers);
    return;
  }
  linked_list_insert(&exit_listener_processes, &current_task()->process->exit_listener_member, current_task()->process);
  current_task()->process->exit_listener = true;
}
void syscall_get_exited_pid(union syscall_args* args) {
  if (!current_task()->process->exit_listener) {
    puts("Not listening to process exits");
    terminate_current_task(&args->registers);
    return;
  }
  struct exited_pid* exited_pid = (struct exited_pid*) current_task()->process->exited_pids_list.first;
  if (exited_pid) {
    args->return_value = exited_pid->pid;
    linked_list_remove(&current_task()->process->exited_pids_list, &exited_pid->list_member);
    free(exited_pid);
  } else {
    args->return_value = 0;
  }
}
