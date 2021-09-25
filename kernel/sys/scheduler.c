#include <panic.h>
#include <sys/lapic.h>
#include <sys/task.h>

struct linked_list scheduler_list[PRIORITY_IDLE + 1];

void scheduler(struct isr_registers* registers) {
  size_t max_priority = current_task()->priority;
  if (current_task()->blocked) {
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
  set_lapic_timer(10);
}
void schedule_task(struct task* new_task, struct isr_registers* registers) {
  insert_linked_list(&scheduler_list[new_task->priority], &new_task->list_member);
  if (registers && new_task->priority < current_task()->priority) {
    scheduler(registers);
  }
}
