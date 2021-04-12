#pragma once
#include <linked_list.h>
#include <sys/syscall.h>
#define IPC_CALL_MEMORY_SHARING 0x80
#define IPC_CALL_MEMORY_SHARING_RW_MASK 0x40

enum {
  IPC_ERR_PLACEHOLDER,
  IPC_ERR_INVALID_PID,
  IPC_ERR_RETRY
};

extern struct linked_list syscall_processes;

void syscall_block_ipc_call(union syscall_args*);
void syscall_get_ipc_caller_capabilities(union syscall_args*);
void syscall_handle_ipc(union syscall_args*);
void syscall_is_caller_child(union syscall_args*);
void syscall_register_ipc(union syscall_args*);
void syscall_return_ipc(union syscall_args*);
void syscall_unblock_ipc_call(union syscall_args*);
void syscall_wait_ipc(union syscall_args*);
