#include <capability.h>
#include <ipccalls.h>
#include <linked_list.h>
#include <priority.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <syslog.h>

struct process {
  struct linked_list_member list_member;
  pid_t pid;
  struct linked_list args_list;
  struct linked_list envs_list;
};
struct argument {
  struct linked_list_member list_member;
  size_t num;
  size_t size;
  void* value;
};

struct linked_list processes_list;

static struct process* get_process(pid_t pid) {
  for (struct linked_list_member* member = processes_list.first; member; member = member->next) {
    struct process* proc = member->node;
    if (proc->pid == pid) {
      return proc;
    }
  }
  return 0;
}
static struct argument* get_argument(struct process* proc, bool env, size_t num) {
  if (proc) {
    struct linked_list_member* first;
    if (env) {
      first = proc->envs_list.first;
    } else {
      first = proc->args_list.first;
    }
    for (struct linked_list_member* member = first; member; member = member->next) {
      struct argument* arg = member->node;
      if (arg->num == num) {
        return arg;
      }
    }
  }
  return 0;
}
static int64_t get_num_args_handler(uint64_t env, __attribute__((unused)) uint64_t arg1, __attribute__((unused)) uint64_t arg2, __attribute__((unused)) uint64_t arg3, __attribute__((unused)) uint64_t arg4) {
  if (env >= 2) {
    syslog(LOG_DEBUG, "Argument out of range");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  struct process* proc = get_process(get_ipc_caller_pid());
  if (proc) {
    if (env) {
      return proc->envs_list.size;
    } else {
      return proc->args_list.size;
    }
  }
  return 0;
}
static int64_t get_arg_size_handler(uint64_t env, uint64_t num, __attribute__((unused)) uint64_t arg2, __attribute__((unused)) uint64_t arg3, __attribute__((unused)) uint64_t arg4) {
  if (env >= 2) {
    syslog(LOG_DEBUG, "Argument out of range");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  struct argument* arg = get_argument(get_process(get_ipc_caller_pid()), env, num);
  if (arg) {
    return arg->size;
  } else {
    syslog(LOG_DEBUG, "Invalid argument number");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
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
  struct process* proc = get_process(get_ipc_caller_pid());
  struct argument* arg = get_argument(proc, env, num);
  if (arg) {
    if (arg->size != size) {
      syslog(LOG_DEBUG, "Invalid argument size");
      return -IPC_ERR_INVALID_ARGUMENTS;
    }
    memcpy((void*) address, arg->value, size);
    if (!noremove) {
      free(arg->value);
      if (env) {
        linked_list_remove(&proc->envs_list, &arg->list_member);
      } else {
        linked_list_remove(&proc->args_list, &arg->list_member);
      }
      free(arg);
      if (!proc->args_list.first && !proc->envs_list.first) {
        linked_list_remove(&processes_list, &proc->list_member);
        free(proc);
      }
    }
    return 0;
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
  struct process* proc = get_process(pid);
  if (!proc) {
    proc = calloc(1, sizeof(struct process));
    proc->pid = pid;
    linked_list_insert(&processes_list, &proc->list_member, proc);
  }
  struct argument* arg = calloc(1, sizeof(struct argument));
  if (env) {
    arg->num = proc->envs_list.size;
  } else {
    arg->num = proc->args_list.size;
  }
  arg->size = size;
  arg->value = strdup(buffer);
  if (env) {
    linked_list_insert(&proc->envs_list, &arg->list_member, arg);
  } else {
    linked_list_insert(&proc->args_list, &arg->list_member, arg);
  }
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
      struct process* proc = get_process(pid);
      if (!proc) {
        continue;
      }
      struct linked_list* lists[] = {&proc->args_list, &proc->envs_list};
      for (size_t i = 0; i < 2; i++) {
        struct linked_list_member* next_member;
        for (struct linked_list_member* member = lists[i]->first; member; member = next_member) {
          next_member = member->next;
          struct argument* arg = member->node;
          free(arg->value);
          free(arg);
        }
      }
      linked_list_remove(&processes_list, &proc->list_member);
      free(proc);
    }
  }
}
