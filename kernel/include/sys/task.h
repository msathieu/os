#pragma once
#include <cpu/isr.h>
#include <sys/process.h>

enum {
  PRIORITY_SYSTEM_HIGH,
  PRIORITY_SYSTEM_NORMAL,
  PRIORITY_SYSTEM_LOW,
  PRIORITY_ROOT_HIGH,
  PRIORITY_ROOT_NORMAL,
  PRIORITY_ROOT_LOW,
  PRIORITY_USER_HIGH,
  PRIORITY_USER_NORMAL,
  PRIORITY_USER_LOW,
  PRIORITY_IDLE
};
struct task {
  struct linked_list_member list_member;
  struct process* process;
  struct isr_registers registers;
  bool blocked;
  struct task* servicing_syscall_requester;
  struct process* spawned_process;
  bool sharing_memory;
  size_t sleep_until;
  int priority;
  _Alignas(16) uint8_t fxsave_region[512];
  uint64_t mappings_bitset[PAGING_PHYSICAL_MAPPINGS_SIZE / 0x1000 / 64];
};

extern struct task* current_task;

void block_current_task(struct isr_registers*);
struct task* create_task(struct process*);
void destroy_task(struct task*);
_Noreturn void setup_multitasking(void);
void switch_task(struct task*, struct isr_registers*);
void terminate_current_task(struct isr_registers*);
