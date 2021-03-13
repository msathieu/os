#pragma once
#include <sys/task.h>

void schedule_task(struct task*, struct isr_registers*);
void scheduler(struct isr_registers*);

extern struct linked_list scheduler_list[PRIORITY_IDLE + 1];
