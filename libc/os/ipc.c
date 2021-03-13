#include <__/syscall.h>
#include <ipc.h>
#include <sched.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

ipc_handler ipc_handlers[256];

void register_ipc(bool accepts_shared_memory) {
  _syscall(_SYSCALL_REGISTER_IPC, 1, accepts_shared_memory, 0, 0, 0);
}
void unregister_ipc(void) {
  _syscall(_SYSCALL_REGISTER_IPC, 0, 0, 0, 0, 0);
}
int64_t ipc_common(uint8_t syscall, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  if (!ipc_handlers[syscall]) {
    return -IPC_ERR_INVALID_SYSCALL;
  }
  return ipc_handlers[syscall](arg0, arg1, arg2, arg3, arg4);
}
bool has_ipc_caller_capability(int namespace, int capability) {
  size_t capabilities = _syscall(_SYSCALL_GET_IPC_CALLER_CAPABILITIES, namespace, 0, 0, 0, 0);
  if (capabilities & (1 << capability)) {
    return 1;
  }
  return 0;
}
pid_t get_ipc_caller_pid(void) {
  return _syscall(_SYSCALL_GET_PID, 1, 0, 0, 0, 0);
}
uid_t get_ipc_caller_uid(void) {
  return _syscall(_SYSCALL_GET_UID, 1, 0, 0, 0, 0);
}
int64_t send_pid_ipc_call(pid_t pid, uint8_t syscall, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  uintptr_t buffer = arg3;
  if (syscall & IPC_CALL_MEMORY_SHARING) {
    buffer = (uintptr_t) aligned_alloc(0x1000, (arg4 + 0xfff) / 0x1000 * 0x1000);
    memcpy((void*) buffer, (void*) arg3, arg4);
  }
  while (1) {
    int64_t return_value = _syscall(pid << 8 | syscall, arg0, arg1, arg2, buffer, arg4);
    if (return_value != -IPC_ERR_RETRY) {
      if (syscall & IPC_CALL_MEMORY_SHARING) {
        if (syscall & IPC_CALL_MEMORY_SHARING_RW_MASK) {
          memcpy((void*) arg3, (void*) buffer, arg4);
        }
        free((void*) buffer);
      }
      return return_value;
    }
    sched_yield();
  }
}
void register_ipc_name(const char* name) {
  char process_name[5];
  strncpy(process_name, name, 5);
  int64_t return_value = send_pid_ipc_call(2, 0, process_name[0], process_name[1], process_name[2], process_name[3], process_name[4]);
  if (return_value == -IPC_ERR_INVALID_PID) {
    sched_yield();
    return register_ipc_name(name);
  }
  if (return_value < 0) {
    exit(1);
  }
}
int64_t send_ipc_call(const char* name, uint8_t syscall, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  static char process_name[5] = "";
  static int64_t pid = 0;
  if (strncmp(process_name, name, 5)) {
    strncpy(process_name, name, 5);
    pid = 0;
  }
  bool first_iteration = 1;
  while (1) {
    if (!first_iteration || !pid) {
      pid = send_pid_ipc_call(2, 1, process_name[0], process_name[1], process_name[2], process_name[3], process_name[4]);
    }
    first_iteration = 0;
    if (pid > 0) {
      int64_t return_value = send_pid_ipc_call(pid, syscall, arg0, arg1, arg2, arg3, arg4);
      if (return_value != -IPC_ERR_INVALID_PID) {
        return return_value;
      }
    }
    sched_yield();
  }
}
void ipc_unblock(pid_t pid) {
  _syscall(_SYSCALL_UNBLOCK_IPC_CALL, pid, 0, 0, 0, 0);
}
bool is_caller_child(void) {
  return _syscall(_SYSCALL_IS_CALLER_CHILD, 0, 0, 0, 0, 0);
}
pid_t get_caller_spawned_pid(void) {
  return _syscall(_SYSCALL_GET_CALLER_SPAWNED_PID, 0, 0, 0, 0, 0);
}
