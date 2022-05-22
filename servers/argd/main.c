#include <capability.h>
#include <ipccalls.h>
#include <linked_list.h>
#include <priority.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <syslog.h>

struct argument {
  struct linked_list_member list_member;
  pid_t pid;
  size_t num;
  size_t size;
  void* value;
  bool is_env;
};

struct linked_list args_list;

static int64_t get_num_args_handler(uint64_t env, __attribute__((unused)) uint64_t arg1, __attribute__((unused)) uint64_t arg2, __attribute__((unused)) uint64_t arg3, __attribute__((unused)) uint64_t arg4) {
  if (env >= 2) {
    syslog(LOG_DEBUG, "Argument out of range");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t caller_pid = get_ipc_caller_pid();
  size_t num_args = 0;
  for (struct linked_list_member* member = args_list.first; member; member = member->next) {
    struct argument* arg = member->node;
    if (arg->pid == caller_pid && arg->is_env == env) {
      num_args++;
    }
  }
  return num_args;
}
static int64_t get_arg_size_handler(uint64_t env, uint64_t num, __attribute__((unused)) uint64_t arg2, __attribute__((unused)) uint64_t arg3, __attribute__((unused)) uint64_t arg4) {
  if (env >= 2) {
    syslog(LOG_DEBUG, "Argument out of range");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t caller_pid = get_ipc_caller_pid();
  for (struct linked_list_member* member = args_list.first; member; member = member->next) {
    struct argument* arg = member->node;
    if (arg->pid == caller_pid && arg->num == num && arg->is_env == env) {
      return arg->size;
    }
  }
  syslog(LOG_DEBUG, "Invalid argument number");
  return -IPC_ERR_INVALID_ARGUMENTS;
}
static int64_t get_arg_handler(uint64_t env, uint64_t num, uint64_t noremove, uint64_t address, uint64_t size) {
  if (env >= 2) {
    syslog(LOG_DEBUG, "Argument out of range");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (noremove >= 2) {
    syslog(LOG_DEBUG, "Argument out of range");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t caller_pid = get_ipc_caller_pid();
  for (struct linked_list_member* member = args_list.first; member; member = member->next) {
    struct argument* arg = member->node;
    if (arg->pid == caller_pid && arg->num == num && arg->is_env == env) {
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
static int64_t add_arg_handler(uint64_t env, __attribute__((unused)) uint64_t arg1, __attribute__((unused)) uint64_t arg2, uint64_t address, uint64_t size) {
  if (env >= 2) {
    syslog(LOG_DEBUG, "Argument out of range");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t pid = get_caller_spawned_pid();
  if (!pid) {
    syslog(LOG_DEBUG, "Not currently spawning a process");
    return -IPC_ERR_PROGRAM_DEFINED;
  }
  char* buffer = (char*) address;
  if (buffer[size - 1]) {
    syslog(LOG_DEBUG, "Buffer isn't null terminated");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (env && !strchr(buffer, '=')) {
    syslog(LOG_DEBUG, "Buffer doesn't contain separator");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  struct argument* arg = calloc(1, sizeof(struct argument));
  arg->pid = pid;
  arg->is_env = env;
  size_t num_args = 0;
  for (struct linked_list_member* member = args_list.first; member; member = member->next) {
    struct argument* arg_i = member->node;
    if (arg_i->pid == pid && arg_i->is_env == env) {
      num_args++;
    }
  }
  arg->num = num_args;
  arg->size = size;
  arg->value = strdup(buffer);
  insert_linked_list(&args_list, &arg->list_member, arg);
  return 0;
}
int main(void) {
  change_priority(PRIORITY_SYSTEM_HIGH);
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  listen_exits();
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_LISTEN_EXITS);
  register_ipc();
  register_ipc_call(IPC_ARGD_GET_NUM, get_num_args_handler, 1);
  register_ipc_call(IPC_ARGD_GET_SIZE, get_arg_size_handler, 2);
  register_ipc_call(IPC_ARGD_ADD, add_arg_handler, 1);
  register_ipc_call(IPC_ARGD_GET, get_arg_handler, 3);
  while (true) {
    handle_ipc();
    pid_t pid;
    while ((pid = get_exited_pid())) {
      struct argument* next_arg;
      for (struct argument* arg = (struct argument*) args_list.first; arg; arg = next_arg) {
        next_arg = (struct argument*) arg->list_member.next;
        if (arg->pid == pid) {
          free(arg->value);
          remove_linked_list(&args_list, &arg->list_member);
          free(arg);
        }
      }
    }
  }
}
