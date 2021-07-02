#pragma once
#include <cpu/isr.h>
#include <stddef.h>

union syscall_args {
  struct isr_registers registers;
  struct {
    int64_t return_value;
    uint64_t unused;
    uint64_t arg2;
    uint64_t arg1;
    uint64_t arg0;
    uint64_t syscall;
    uint64_t unused2;
    uint64_t arg3;
    uint64_t arg4;
  };
};

extern struct process* ioports_process[0x10000];
extern struct process* isa_irqs_process[16];
extern struct process* exit_listener_processes[64];

void syscall_change_memory_permissions(union syscall_args*);
void syscall_change_priority(union syscall_args*);
void syscall_drop_capabilities(union syscall_args*);
void syscall_get_exited_pid(union syscall_args*);
void syscall_get_pid(union syscall_args*);
void syscall_get_uid(union syscall_args*);
void syscall_grant_capabilities(union syscall_args*);
void syscall_has_arguments(union syscall_args*);
void syscall_listen_exits(union syscall_args*);
void syscall_map_memory(union syscall_args*);
void syscall_set_spawned_uid(union syscall_args*);
void syscall_sleep(union syscall_args*);
void syscall_spawn_process(union syscall_args*);
void syscall_start_process(union syscall_args*);
void syscall_wait(union syscall_args*);

void syscall_access_ioport(union syscall_args*);
void syscall_clear_irqs(union syscall_args*);
void syscall_get_acpi_revision(union syscall_args*);
void syscall_get_acpi_table(union syscall_args*);
void syscall_get_fb_info(union syscall_args*);
void syscall_grant_ioport(union syscall_args*);
void syscall_map_phys_memory(union syscall_args*);
void syscall_register_irq(union syscall_args*);
void syscall_wait_irq(union syscall_args*);
