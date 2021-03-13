#include <capability.h>
#include <ipc.h>
#include <linked_list.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

struct fd {
  struct linked_list_member list_member;
  size_t fd;
  size_t file_num;
  size_t position;
};
struct process {
  struct linked_list_member list_member;
  pid_t pid;
  struct linked_list fd_list;
  size_t next_fd;
};

static pid_t svfsd_pid;
static struct linked_list process_list;

static int64_t mount_handler(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  if (arg0 || arg1 || arg2 || arg3 || arg4) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (!has_ipc_caller_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_VFSD_MOUNT)) {
    syslog(LOG_DEBUG, "No permission to mount filesystem");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  svfsd_pid = get_caller_spawned_pid();
  return 0;
}
static int64_t open_file_handler(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size) {
  if (arg0 || arg1 || arg2) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (!svfsd_pid) {
    return -IPC_ERR_RETRY;
  }
  char* buffer = malloc(size);
  memcpy(buffer, (void*) address, size);
  if (buffer[size - 1]) {
    free(buffer);
    syslog(LOG_DEBUG, "Path isn't null terminated");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (buffer[0] != '/') {
    free(buffer);
    syslog(LOG_DEBUG, "Path isn't absolute");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  long file_num = send_pid_ipc_call(svfsd_pid, IPC_CALL_MEMORY_SHARING, 0, 0, 0, (uintptr_t) buffer + 1, size - 1);
  free(buffer);
  if (file_num < 0) {
    return -IPC_ERR_PROGRAM_DEFINED;
  }
  pid_t caller_pid = get_ipc_caller_pid();
  struct process* process = 0;
  for (struct process* loop_process = (struct process*) process_list.first; loop_process; loop_process = (struct process*) loop_process->list_member.next) {
    if (loop_process->pid == caller_pid) {
      process = loop_process;
      break;
    }
  }
  if (!process) {
    process = calloc(1, sizeof(struct process));
    process->pid = caller_pid;
    insert_linked_list(&process_list, &process->list_member);
  }
  struct fd* fd = calloc(1, sizeof(struct fd));
  fd->file_num = file_num;
  fd->fd = process->next_fd++;
  insert_linked_list(&process->fd_list, &fd->list_member);
  return fd->fd;
}
static int64_t close_file_handler(uint64_t fd_num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  if (arg1 || arg2 || arg3 || arg4) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t caller_pid = get_ipc_caller_pid();
  for (struct process* process = (struct process*) process_list.first; process; process = (struct process*) process->list_member.next) {
    if (process->pid == caller_pid) {
      for (struct fd* fd = (struct fd*) process->fd_list.first; fd; fd = (struct fd*) fd->list_member.next) {
        if (fd->fd == fd_num) {
          remove_linked_list(&process->fd_list, &fd->list_member);
          free(fd);
          return 0;
        }
      }
      break;
    }
  }
  syslog(LOG_DEBUG, "File descriptor doesn't exist");
  return -IPC_ERR_INVALID_ARGUMENTS;
}
static int64_t read_file_handler(uint64_t fd_num, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size) {
  if (arg1 || arg2) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t caller_pid = get_ipc_caller_pid();
  for (struct process* process = (struct process*) process_list.first; process; process = (struct process*) process->list_member.next) {
    if (process->pid == caller_pid) {
      for (struct fd* fd = (struct fd*) process->fd_list.first; fd; fd = (struct fd*) fd->list_member.next) {
        if (fd->fd == fd_num) {
          send_pid_ipc_call(svfsd_pid, IPC_CALL_MEMORY_SHARING_RW, fd->file_num, fd->position, 0, address, size);
          fd->position += size;
          return 0;
        }
      }
      break;
    }
  }
  syslog(LOG_DEBUG, "File descriptor doesn't exist");
  return -IPC_ERR_INVALID_ARGUMENTS;
}
static int64_t seek_file_handler(uint64_t fd_num, uint64_t position, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  if (arg2 || arg3 || arg4) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t caller_pid = get_ipc_caller_pid();
  for (struct process* process = (struct process*) process_list.first; process; process = (struct process*) process->list_member.next) {
    if (process->pid == caller_pid) {
      for (struct fd* fd = (struct fd*) process->fd_list.first; fd; fd = (struct fd*) fd->list_member.next) {
        if (fd->fd == fd_num) {
          fd->position = position;
          return 0;
        }
      }
      break;
    }
  }
  syslog(LOG_DEBUG, "File descriptor doesn't exist");
  return -IPC_ERR_INVALID_ARGUMENTS;
}
int main(void) {
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  register_ipc(1);
  ipc_handlers[0] = mount_handler;
  ipc_handlers[1] = close_file_handler;
  ipc_handlers[2] = seek_file_handler;
  ipc_handlers[IPC_CALL_MEMORY_SHARING] = open_file_handler;
  ipc_handlers[IPC_CALL_MEMORY_SHARING_RW] = read_file_handler;
  while (1) {
    handle_ipc();
  }
}
