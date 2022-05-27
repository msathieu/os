#include <panic.h>
#include <sys/lapic.h>
#include <sys/scheduler.h>

struct linked_list scheduler_list[PRIORITY_IDLE + 1];

void scheduler(struct isr_registers* registers) {
  size_t max_priority = current_task()->priority;
  if (max_priority == PRIORITY_IDLE) {
    max_priority--;
  }
  if (current_task()->blocked) {
    max_priority = PRIORITY_IDLE;
  }
loop:
  for (size_t i = 0; i <= max_priority; i++) {
    if (scheduler_list[i].first) {
      struct task* task = scheduler_list[i].first->node;
      linked_list_remove(&scheduler_list[i], scheduler_list[i].first);
      if (task->process->should_exit) {
        destroy_task(task);
        goto loop;
      }
      if (task->priority == PRIORITY_IDLE && task->cpu != get_current_lapic_id()) {
        schedule_task(task, 0);
        goto loop;
      }
      switch_task(task, registers);
      return;
    }
  }
  if (!is_core_idle[get_current_lapic_id()]) {
    set_lapic_timer(10);
  }
}
void schedule_task(struct task* new_task, struct isr_registers* registers) {
  linked_list_insert(&scheduler_list[new_task->priority], &new_task->state_list_member, new_task);
  if (registers && new_task->priority < current_task()->priority) {
    scheduler(registers);
  }
  if (ncore_idle) {
    for (size_t i = 0; i < PRIORITY_IDLE; i++) {
      if (scheduler_list[i].first) {
        for (size_t j = 0; j < 256; j++) {
          if (is_core_idle[j]) {
            smp_wakeup_core(j);
          }
        }
        return;
      }
    }
  }
}
