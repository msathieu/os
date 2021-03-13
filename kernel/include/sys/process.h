#pragma once
#include <linked_list.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum {
  CAP_MANAGE,
  CAP_IOPORT,
  CAP_IRQ,
  CAP_MAP_MEMORY,
  CAP_GET_FB_INFO,
  CAP_PRIORITY
};

struct process {
  struct linked_list_member list_member;
  size_t pid;
  size_t uid;
  size_t ntasks;
  struct paging_table* address_space;
  bool accepts_syscalls;
  bool accepts_shared_memory;
  struct task* syscall_handler;
  struct linked_list syscall_queue;
  size_t file_i;
  bool ioports_assigned;
  struct task* irq_handler;
  bool irqs_assigned;
  size_t capabilities[64];
  uintptr_t physical_mappings_addr;
  uintptr_t last_physical_mappings_addr;
  struct process* parent;
  struct process* first_child;
  struct process* next_sibling;
  bool exited;
  struct task* waiting_task;
  struct linked_list blocked_ipc_calls_queue;
  bool has_arguments;
  bool has_environment_vars;
};

struct process* create_process(void);
void destroy_process(struct process*);
bool has_process_capability(struct process*, int);
void remove_process(struct process*);
