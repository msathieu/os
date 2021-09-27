#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#define IPC_CALL_MEMORY_SHARING 0x80
#define IPC_CALL_MEMORY_SHARING_RW 0xc0
#define IPC_CALL_MEMORY_SHARING_RW_MASK 0x40

typedef int64_t (*ipc_handler)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

enum {
  IPC_ERR_PLACEHOLDER,
  IPC_ERR_INVALID_PID,
  IPC_ERR_BLOCK,
  IPC_ERR_INVALID_SYSCALL,
  IPC_ERR_INVALID_ARGUMENTS,
  IPC_ERR_INSUFFICIENT_PRIVILEGE,
  IPC_ERR_PROGRAM_DEFINED
};

pid_t get_caller_spawned_pid(void);
pid_t get_ipc_caller_pid(void);
uid_t get_ipc_caller_uid(void);
void handle_ipc(void);
bool has_ipc_caller_capability(int namespace, int capability);
void ipc_set_started(void);
void ipc_unblock(pid_t pid);
bool is_caller_child(void);
void register_ipc(bool);
void register_ipc_name(const char* name);
int64_t send_ipc_call(const char* name, uint8_t syscall, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4);
int64_t send_pid_ipc_call(pid_t pid, uint8_t syscall, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4);
void unregister_ipc(void);

extern ipc_handler ipc_handlers[256];
