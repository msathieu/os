#pragma once
#include <cpu/paging.h>
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
  CAP_PRIORITY,
  CAP_ACPI,
  CAP_LOG,
  CAP_LISTEN_EXITS,
  CAP_REGISTER_IPC_NAME
};

struct process {
  struct linked_list_member ipc_list_member;
  struct linked_list_member exit_listener_member;
  size_t pid;
  size_t uid;
  size_t ntasks;
  struct paging_table* address_space;
  bool accepts_syscalls;
  struct task* syscall_handler;
  struct linked_list syscall_queue;
  size_t file_i;
  bool ioports_assigned;
  struct task* irq_handler;
  bool irqs_assigned;
  size_t capabilities[64];
  struct process* parent;
  struct process* first_child;
  struct process* next_sibling;
  bool exited;
  struct task* waiting_task;
  struct linked_list blocked_ipc_calls_queue;
  bool has_arguments;
  bool has_environment_vars;
  uint64_t mappings_bitset[PAGING_PHYSICAL_MAPPINGS_SIZE / 0x1000 / 64];
  struct linked_list exited_pids_list;
  bool exit_listener;
  bool should_exit;
  char ipc_name[5];
};
struct exited_pid {
  struct linked_list_member list_member;
  size_t pid;
};

struct process* create_process(bool);
void destroy_process(struct process*);
bool has_process_capability(struct process*, int);
void remove_process(struct process*);
struct process* spawn_child(bool);
