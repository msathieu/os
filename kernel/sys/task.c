#include <bitset.h>
#include <cpu/paging.h>
#include <cpu/smp.h>
#include <elf.h>
#include <panic.h>
#include <stdlib.h>
#include <string.h>
#include <struct.h>
#include <sys/hpet.h>
#include <sys/lapic.h>
#include <sys/lock.h>
#include <sys/madt.h>
#include <sys/scheduler.h>

static struct task* current_tasks[256];

struct task* current_task(void) {
  return current_tasks[get_current_lapic_id()];
}
void switch_task(struct task* task, struct isr_registers* isr_registers) {
  current_task()->time_used += get_time() - current_task()->start_time;
  switch_pml4(task->process->address_space);
  memcpy(&current_task()->registers, isr_registers, sizeof(struct isr_registers));
  memcpy(isr_registers, &task->registers, sizeof(struct isr_registers));
  if (!current_task()->blocked) {
    schedule_task(current_task(), 0);
  }
  asm volatile("fxsave (%0)"
               :
               : "r"(current_task()->fxsave_region));
  asm volatile("fxrstor (%0)"
               :
               : "r"(task->fxsave_region));
  asm volatile("wrmsr"
               :
               : "c"(0xc0000100), "a"(task->fs), "d"(task->fs >> 32));
  current_tasks[get_current_lapic_id()] = task;
  current_task()->start_time = get_time();
  set_lapic_timer(10);
}
struct task* create_task(struct process* process) {
  struct task* task = calloc(1, sizeof(struct task));
  task->process = process;
  task->process->ntasks++;
  if (has_process_capability(process, CAP_PRIORITY)) {
    task->priority = PRIORITY_SYSTEM_NORMAL;
  } else if (!task->process->uid) {
    task->priority = PRIORITY_ROOT_NORMAL;
  } else {
    task->priority = PRIORITY_USER_NORMAL;
  }
  return task;
}
void destroy_task(struct task* task) {
  if (task->sharing_memory) {
    switch_pml4(task->process->address_space);
    for (size_t i = 0; i < PAGING_PHYSICAL_MAPPINGS_SIZE / 0x1000 / 64; i++) {
      if (!task->mappings_bitset[i]) {
        continue;
      }
      for (size_t j = 0; j < 64; j++) {
        if (bitset_test(task->mappings_bitset, i * 64 + j)) {
          unmap_page(PAGING_USER_PHYS_MAPPINGS_START + (i * 64 + j) * 0x1000);
          bitset_clear(task->process->mappings_bitset, i * 64 + j);
        }
      }
    }
    switch_pml4(current_task()->process->address_space);
  }
  if (task->servicing_syscall_requester) {
    destroy_task(task->servicing_syscall_requester);
  }
  if (task->spawned_process) {
    destroy_process(task->spawned_process);
  }
  task->process->ntasks--;
  if (!task->process->ntasks) {
    destroy_process(task->process);
  }
  free(task);
}
void block_current_task(struct isr_registers* registers) {
  current_task()->blocked = 1;
  scheduler(registers);
}
void terminate_current_task(struct isr_registers* registers) {
  struct task* task = current_task();
  block_current_task(registers);
  destroy_task(task);
}
static _Noreturn void idle(void) {
  while (1) {
    asm volatile("hlt");
  }
}
_Noreturn void setup_multitasking(void) {
  uint64_t rflags;
  asm volatile("pushf; mov (%%rsp), %0; popf"
               : "=r"(rflags));
  asm volatile("cli");
  struct process* idle_process = create_process(0);
  current_tasks[get_current_lapic_id()] = create_task(create_process(0));
  switch_pml4(current_task()->process->address_space);
  current_task()->process->capabilities[0] = 1 << CAP_MANAGE;
  current_task()->priority = PRIORITY_SYSTEM_NORMAL;
  for (size_t i = 0; i < madt_num_lapics; i++) {
    struct task* idle_task = create_task(idle_process);
    idle_task->priority = PRIORITY_IDLE;
    idle_task->registers.cs = 8;
    idle_task->registers.rsp = (uintptr_t) malloc(0x2000) + 0x2000;
    idle_task->registers.rip = (uintptr_t) idle;
    idle_task->registers.rflags = rflags;
    idle_task->cpu = madt_lapics[i]->lapic_id;
    schedule_task(idle_task, 0);
  }
  acquire_lock();
  aps_jmp_user = 1;
  for (size_t i = 0; i < 64; i++) {
    if (!strcmp((char*) loader_struct.files[i].name, "init")) {
      current_task()->start_time = get_time();
      set_lapic_timer(10);
      load_elf(i);
    }
  }
  panic("No init loaded");
}
