#include <capability.h>
#include <ctype.h>
#include <fcntl.h>
#include <ipccalls.h>
#include <linked_list.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

struct fd {
  bool exists;
  size_t mount;
  size_t file_num;
  size_t position;
  size_t flags;
};
struct process {
  struct linked_list_member list_member;
  pid_t pid;
  size_t nfds;
  struct fd fds[];
};
struct mount {
  const char* path;
  pid_t pid;
};

static struct mount mounts[512];
static struct linked_list process_list;
static bool ready;
static size_t blocked_calls;

static int64_t mount_handler(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size) {
  if (arg0 || arg1 || arg2) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (!has_ipc_caller_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_VFSD_MOUNT)) {
    syslog(LOG_DEBUG, "No permission to mount filesystem");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  if (size == 1) {
    syslog(LOG_DEBUG, "No mount path specified");
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
    syslog(LOG_DEBUG, "Path isn't null terminated");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (buffer[0] != '/') {
    free(buffer);
    syslog(LOG_DEBUG, "Path isn't absolute");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (buffer[size - 2] != '/') {
    free(buffer);
    syslog(LOG_DEBUG, "Path isn't a directory");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  for (size_t i = 0; i < size - 1; i++) {
    if (!isalnum(buffer[i]) && buffer[i] != '/') {
      free(buffer);
      syslog(LOG_DEBUG, "Path contains invalid characters");
      return -IPC_ERR_INVALID_ARGUMENTS;
    }
  }
  for (size_t i = 0; i < 512; i++) {
    if (!mounts[i].path) {
      mounts[i].path = buffer;
      mounts[i].pid = pid;
      return 0;
    }
  }
  free(buffer);
  syslog(LOG_CRIT, "Reached maximum number of mounts");
  return -IPC_ERR_PROGRAM_DEFINED;
}
static int64_t open_file_handler(uint64_t flags, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size) {
  if (arg1 || arg2) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (!ready) {
    blocked_calls++;
    return -IPC_ERR_BLOCK;
  }
  size_t access_flags = flags & (O_RDONLY | O_RDWR | O_WRONLY);
  if (access_flags != O_RDONLY && access_flags != O_RDWR && access_flags != O_WRONLY) {
    syslog(LOG_DEBUG, "Invalid flags");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if ((flags & O_RDONLY) && (flags & O_TRUNC)) {
    syslog(LOG_DEBUG, "Invalid flags");
    return -IPC_ERR_INVALID_ARGUMENTS;
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
  int mount_i = -1;
  size_t mount_size = 0;
  for (size_t i = 0; i < 512; i++) {
    if (mounts[i].path && strlen(mounts[i].path) > mount_size && !strncmp(mounts[i].path, buffer, strlen(mounts[i].path))) {
      mount_i = i;
      mount_size = strlen(mounts[i].path);
    }
  }
  if (mount_i == -1) {
    free(buffer);
    syslog(LOG_ERR, "No root filesystem mounted");
    return -IPC_ERR_PROGRAM_DEFINED;
  }
  long file_num = -IPC_ERR_INVALID_PID;
  while (file_num == -IPC_ERR_INVALID_PID) {
    file_num = send_pid_ipc_call(mounts[mount_i].pid, IPC_VFSD_FS_OPEN, flags, 0, 0, (uintptr_t) buffer + mount_size, size - mount_size);
    if (file_num == -IPC_ERR_INVALID_PID) {
      sched_yield();
    }
  }
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
    process = calloc(1, sizeof(struct process) + 128 * sizeof(struct fd));
    process->pid = caller_pid;
    process->nfds = 128;
    process->fds[0].exists = 1;
    process->fds[1].exists = 1;
    process->fds[2].exists = 1;
    insert_linked_list(&process_list, &process->list_member);
  }
  struct fd* fd = 0;
  size_t fd_i;
  for (size_t i = 0; i < process->nfds; i++) {
    if (!process->fds[i].exists) {
      fd = &process->fds[i];
      fd_i = i;
      break;
    }
  }
  if (!fd) {
    fd_i = process->nfds;
    process->nfds += 128;
    process = realloc(process, sizeof(struct process) + process->nfds * sizeof(struct fd));
    memset(process + sizeof(struct process) + (process->nfds - 128) * sizeof(struct fd), 0, 128 * sizeof(struct fd));
    fd = &process->fds[fd_i];
  }
  fd->exists = 1;
  fd->mount = mount_i;
  fd->file_num = file_num;
  fd->flags = flags;
  return fd_i;
}
static int64_t close_file_handler(uint64_t fd_num, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  if (arg1 || arg2 || arg3 || arg4) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t caller_pid = get_ipc_caller_pid();
  for (struct process* process = (struct process*) process_list.first; process; process = (struct process*) process->list_member.next) {
    if (process->pid == caller_pid) {
      if (fd_num >= process->nfds || !process->fds[fd_num].exists) {
        syslog(LOG_DEBUG, "File descriptor doesn't exist");
        return -IPC_ERR_INVALID_ARGUMENTS;
      }
      memset(&process->fds[fd_num], 0, sizeof(struct fd));
      return 0;
    }
  }
  syslog(LOG_DEBUG, "File descriptor doesn't exist");
  return -IPC_ERR_INVALID_ARGUMENTS;
}
static int64_t handle_transfer(uint64_t fd_num, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size, bool write) {
  if (arg1 || arg2) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (write && (fd_num == 1 || fd_num == 2)) {
    return send_ipc_call("ttyd", IPC_VFSD_FS_WRITE, 0, 0, 0, address, size);
  }
  pid_t caller_pid = get_ipc_caller_pid();
  for (struct process* process = (struct process*) process_list.first; process; process = (struct process*) process->list_member.next) {
    if (process->pid == caller_pid) {
      if (fd_num >= process->nfds || !process->fds[fd_num].exists) {
        syslog(LOG_DEBUG, "File descriptor doesn't exist");
        return -IPC_ERR_INVALID_ARGUMENTS;
      }
      if (write && !(process->fds[fd_num].flags & (O_RDWR | O_WRONLY))) {
        syslog(LOG_DEBUG, "File not opened for writing");
        return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
      }
      if (!write && !(process->fds[fd_num].flags & (O_RDONLY | O_RDWR))) {
        syslog(LOG_DEBUG, "File not opened for reading");
        return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
      }
      uint8_t call = IPC_VFSD_FS_READ;
      if (write) {
        call = IPC_VFSD_FS_WRITE;
      }
      int64_t return_value = send_pid_ipc_call(mounts[process->fds[fd_num].mount].pid, call, process->fds[fd_num].file_num, process->fds[fd_num].position, 0, address, size);
      if (return_value > 0) {
        process->fds[fd_num].position += return_value;
      }
      return return_value;
    }
  }
  syslog(LOG_DEBUG, "File descriptor doesn't exist");
  return -IPC_ERR_INVALID_ARGUMENTS;
}
static int64_t read_file_handler(uint64_t fd_num, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size) {
  return handle_transfer(fd_num, arg1, arg2, address, size, 0);
}
static int64_t write_file_handler(uint64_t fd_num, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size) {
  return handle_transfer(fd_num, arg1, arg2, address, size, 1);
}
static int64_t seek_file_handler(uint64_t fd_num, uint64_t mode, uint64_t arg_position, uint64_t arg3, uint64_t arg4) {
  if (arg3 || arg4) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (mode >= 2) {
    syslog(LOG_DEBUG, "Argument out of range");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t caller_pid = get_ipc_caller_pid();
  for (struct process* process = (struct process*) process_list.first; process; process = (struct process*) process->list_member.next) {
    if (process->pid == caller_pid) {
      if (fd_num >= process->nfds || !process->fds[fd_num].exists) {
        syslog(LOG_DEBUG, "File descriptor doesn't exist");
        return -IPC_ERR_INVALID_ARGUMENTS;
      }
      long position = arg_position;
      switch (mode) {
      case SEEK_SET:
        process->fds[fd_num].position = position;
        break;
      case SEEK_CUR:
        process->fds[fd_num].position += position;
        break;
      }
      return process->fds[fd_num].position;
    }
  }
  syslog(LOG_DEBUG, "File descriptor doesn't exist");
  return -IPC_ERR_INVALID_ARGUMENTS;
}
static int64_t set_ready_handler(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4) {
  if (arg0 || arg1 || arg2 || arg3 || arg4) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (!has_ipc_caller_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_VFSD_MOUNT)) {
    syslog(LOG_DEBUG, "No permission to set ready flag");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  ready = 1;
  for (size_t i = 0; i < blocked_calls; i++) {
    ipc_unblock(0);
  }
  blocked_calls = 0;
  return 0;
}
int main(void) {
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  register_ipc(1);
  ipc_handlers[IPC_VFSD_CLOSE] = close_file_handler;
  ipc_handlers[IPC_VFSD_SEEK] = seek_file_handler;
  ipc_handlers[IPC_VFSD_SET_READY] = set_ready_handler;
  ipc_handlers[IPC_VFSD_OPEN] = open_file_handler;
  ipc_handlers[IPC_VFSD_MOUNT] = mount_handler;
  ipc_handlers[IPC_VFSD_WRITE] = write_file_handler;
  ipc_handlers[IPC_VFSD_READ] = read_file_handler;
  while (1) {
    handle_ipc();
  }
}
