#include <panic.h>
#include <sys/lapic.h>
#include <sys/task.h>

struct linked_list scheduler_list[PRIORITY_IDLE + 1];

void scheduler(struct isr_registers* registers) {
  size_t max_priority = current_task->priority;
  if (current_task->blocked) {
    max_priority = PRIORITY_IDLE;
  }
loop:
  for (size_t i = 0; i <= max_priority; i++) {
    if (scheduler_list[i].first) {
      struct task* task = (struct task*) scheduler_list[i].first;
      scheduler_list[i].first = scheduler_list[i].first->next;
      if (task->process->should_exit) {
        destroy_task(task);
        goto loop;
      }
      switch_task(task, registers);
      return;
    }
  }
}
void schedule_task(struct task* new_task, struct isr_registers* registers) {
  if (new_task->priority == current_task->priority || (!registers && new_task->priority < current_task->priority)) {
    bool set_timer = 1;
    for (size_t i = 0; i <= (size_t) current_task->priority; i++) {
      if (scheduler_list[i].first) {
        set_timer = 0;
      }
    }
    if (set_timer) {
      set_lapic_timer(10);
    }
  }
  insert_linked_list(&scheduler_list[new_task->priority], &new_task->list_member);
  if (registers && new_task->priority < current_task->priority) {
    scheduler(registers);
  }
}
