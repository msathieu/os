#include <capability.h>
#include <ipccalls.h>
#include <linked_list.h>
#include <priority.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

struct argument {
  struct linked_list_member list_member;
  pid_t pid;
  size_t num;
  size_t size;
  void* value;
};

struct linked_list args_list;

static int64_t get_num_args_handler(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  if (arg0 || arg1 || arg2 || arg3 || arg4) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t caller_pid = get_ipc_caller_pid();
  size_t num_args = 0;
  for (struct argument* arg = (struct argument*) args_list.first; arg; arg = (struct argument*) arg->list_member.next) {
    if (arg->pid == caller_pid) {
      num_args++;
    }
  }
  return num_args;
}
static int64_t get_arg_size_handler(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  if (arg1 || arg2 || arg3 || arg4) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t caller_pid = get_ipc_caller_pid();
  for (struct argument* arg = (struct argument*) args_list.first; arg; arg = (struct argument*) arg->list_member.next) {
    if (arg->pid == caller_pid && arg->num == num) {
      return arg->size;
    }
  }
  syslog(LOG_DEBUG, "Invalid argument number");
  return -IPC_ERR_INVALID_ARGUMENTS;
}
static int64_t get_arg_handler(uint64_t num, uint64_t noremove, uint64_t arg2, uint64_t address, uint64_t size) {
  if (arg2) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (noremove >= 2) {
    syslog(LOG_DEBUG, "Argument out of range");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t caller_pid = get_ipc_caller_pid();
  for (struct argument* arg = (struct argument*) args_list.first; arg; arg = (struct argument*) arg->list_member.next) {
    if (arg->pid == caller_pid && arg->num == num) {
      if (arg->size != size) {
        syslog(LOG_DEBUG, "Invalid argument size");
        return -IPC_ERR_INVALID_ARGUMENTS;
      }
      memcpy((void*) address, arg->value, size);
      if (!noremove) {
        free(arg->value);
        remove_linked_list(&args_list, &arg->list_member);
        free(arg);
      }
      return 0;
    }
  }
  syslog(LOG_DEBUG, "Invalid argument number");
  return -IPC_ERR_INVALID_ARGUMENTS;
}
static int64_t add_arg_handler(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size) {
  if (arg0 || arg1 || arg2) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t pid = get_caller_spawned_pid();
  if (!pid) {
    syslog(LOG_DEBUG, "Not currently spawning a process");
    return -IPC_ERR_PROGRAM_DEFINED;
  }
  char* buffer = malloc(size);
  memcpy(buffer, (void*) address, size);
  if (buffer[size - 1]) {
    free(buffer);
    syslog(LOG_DEBUG, "Buffer isn't null terminated");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  struct argument* arg = calloc(1, sizeof(struct argument));
  arg->pid = pid;
  size_t num_args = 0;
  for (struct argument* arg_i = (struct argument*) args_list.first; arg_i; arg_i = (struct argument*) arg_i->list_member.next) {
    if (arg_i->pid == pid) {
      num_args++;
    }
  }
  arg->num = num_args;
  arg->size = size;
  arg->value = buffer;
  insert_linked_list(&args_list, &arg->list_member);
  return 0;
}
int main(void) {
  change_priority(PRIORITY_SYSTEM_HIGH);
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  register_ipc(1);
  ipc_handlers[IPC_ARGD_GET_NUM] = get_num_args_handler;
  ipc_handlers[IPC_ARGD_GET_SIZE] = get_arg_size_handler;
  ipc_handlers[IPC_ARGD_ADD] = add_arg_handler;
  ipc_handlers[IPC_ARGD_GET] = get_arg_handler;
  while (1) {
    handle_ipc();
  }
}
