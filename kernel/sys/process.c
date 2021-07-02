#include <cpu/paging.h>
#include <panic.h>
#include <stdlib.h>
#include <sys/ioapic.h>
#include <sys/ipc.h>
#include <sys/scheduler.h>

static size_t next_pid;

struct process* create_process(void) {
  if (next_pid > 0xffffffffffffff) {
    panic("Out of available pids");
  }
  struct process* process = calloc(1, sizeof(struct process));
  process->pid = next_pid++;
  process->address_space = create_pml4();
  return process;
}
void destroy_process(struct process* process) {
  if (process->accepts_syscalls) {
    remove_linked_list(&syscall_processes, &process->list_member);
  }
  if (process->syscall_queue.first) {
    struct task* next_task;
    for (struct task* task = (struct task*) process->syscall_queue.first; task; task = next_task) {
      next_task = (struct task*) task->list_member.next;
      destroy_task(task);
    }
  }
  if (process->blocked_ipc_calls_queue.first) {
    struct task* next_task;
    for (struct task* task = (struct task*) process->blocked_ipc_calls_queue.first; task; task = next_task) {
      next_task = (struct task*) task->list_member.next;
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
  }
  destroy_pml4(process->address_space);
  struct process* next_child;
  for (struct process* child = process->first_child; child; child = next_child) {
    next_child = child->next_sibling;
    child->parent = 0;
    if (child->exited) {
      remove_process(child);
    }
  }
  if (!process->parent) {
    remove_process(process);
  } else if (process->parent->waiting_task) {
    struct task* task = process->parent->waiting_task;
    process->parent->waiting_task = 0;
    task->blocked = 0;
    task->registers.rax = process->pid;
    schedule_task(task, 0);
    remove_process(process);
  } else {
    process->exited = 1;
  }
}
void remove_process(struct process* process) {
  if (process->parent) {
    struct process* prev_sibling = 0;
    for (struct process* sibling = process->parent->first_child; sibling; sibling = sibling->next_sibling) {
      if (sibling == process) {
        if (prev_sibling) {
          prev_sibling->next_sibling = process->next_sibling;
        } else {
          process->parent->first_child = process->next_sibling;
        }
        break;
      }
      prev_sibling = sibling;
    }
  }
  free(process);
}
bool has_process_capability(struct process* process, int capability) {
  if (process->capabilities[0] & (1 << CAP_MANAGE | 1 << capability)) {
    return 1;
  } else {
    return 0;
  }
}
