#include <cpu/paging.h>
#include <panic.h>
#include <stdlib.h>
#include <sys/ioapic.h>
#include <sys/ipc.h>
#include <sys/scheduler.h>

static size_t next_pid;

struct process* create_process(bool clone) {
  if (next_pid > 0xffffffffffffff) {
    panic("Out of available pids");
  }
  struct process* process = calloc(1, sizeof(struct process));
  process->pid = next_pid++;
  if (clone) {
    process->address_space = clone_pml4();
  } else {
    process->address_space = create_pml4();
  }
  return process;
}
struct process* spawn_child(bool clone) {
  current_task()->spawned_process = create_process(clone);
  current_task()->spawned_process->parent = current_task()->process;
  insert_linked_list(&current_task()->process->children_list, &current_task()->spawned_process->siblings_list_member, current_task()->spawned_process);
  current_task()->spawned_process->uid = current_task()->process->uid;
  return current_task()->spawned_process;
}
void destroy_process(struct process* process) {
  if (process->pid == 1) {
    panic("init exited");
  }
  if (process->accepts_syscalls) {
    remove_linked_list(&ipc_handling_processes, &process->ipc_list_member);
  }
  if (process->syscall_queue.first) {
    struct linked_list_member* next_member;
    for (struct linked_list_member* member = process->syscall_queue.first; member; member = next_member) {
      struct task* task = member->node;
      next_member = member->next;
      destroy_task(task);
    }
  }
  if (process->blocked_ipc_calls_queue.first) {
    struct linked_list_member* next_member;
    for (struct linked_list_member* member = process->blocked_ipc_calls_queue.first; member; member = next_member) {
      struct task* task = member->node;
      next_member = member->next;
      destroy_task(task);
    }
  }
  if (process->ioports_assigned) {
    for (size_t i = 0; i < 0x10000; i++) {
      if (ioports_process[i] == process) {
        ioports_process[i] = 0;
      }
    }
  }
  if (process->irqs_assigned) {
    for (size_t i = 0; i < 16; i++) {
      if (isa_irqs_process[i] == process) {
        isa_irqs_process[i] = 0;
        unregister_isa_irq(i);
      }
    }
    if (sci_process == process) {
      sci_process = 0;
      isr_handlers[253] = 0;
    }
  }
  struct exited_pid* next_exited_pid;
  for (struct exited_pid* exited_pid = (struct exited_pid*) process->exited_pids_list.first; exited_pid; exited_pid = next_exited_pid) {
    next_exited_pid = (struct exited_pid*) exited_pid->list_member.next;
    free(exited_pid);
  }
  if (process->exit_listener) {
    remove_linked_list(&exit_listener_processes, &process->exit_listener_member);
  }
  destroy_pml4(process->address_space);
  struct linked_list_member* next_member;
  for (struct linked_list_member* member = process->children_list.first; member; member = next_member) {
    struct process* child = member->node;
    next_member = member->next;
    child->parent = 0;
    if (child->exited) {
      remove_process(child);
    }
  }
  for (struct linked_list_member* member = exit_listener_processes.first; member; member = member->next) {
    struct process* listener = member->node;
    struct exited_pid* exited_pid = calloc(1, sizeof(struct exited_pid));
    exited_pid->pid = process->pid;
    insert_linked_list(&listener->exited_pids_list, &exited_pid->list_member, exited_pid);
  }
  if (!process->parent) {
    remove_process(process);
  } else if (process->parent->waiting_task) {
    struct task* task = process->parent->waiting_task;
    process->parent->waiting_task = 0;
    task->blocked = false;
    task->registers.rax = process->pid;
    schedule_task(task, 0);
    remove_process(process);
  } else {
    process->exited = 1;
  }
}
void remove_process(struct process* process) {
  if (process->parent) {
    remove_linked_list(&process->parent->children_list, &process->siblings_list_member);
  }
  free(process);
}
bool has_process_capability(struct process* process, int capability) {
  return __atomic_load_n(&process->capabilities[0], __ATOMIC_SEQ_CST) & (1 << CAP_MANAGE | 1 << capability);
}
