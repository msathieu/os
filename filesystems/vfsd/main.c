#include <capability.h>
#include <ctype.h>
#include <fcntl.h>
#include <ipccalls.h>
#include <linked_list.h>
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <syslog.h>
#include <vfs.h>

struct fs_node {
  struct linked_list_member list_member;
  char name[256];
  size_t inode;
  size_t nfds;
  struct vfs_stat stat;
  struct fs_node* parent;
  struct linked_list children;
};
struct fd {
  bool exists;
  struct fs_node* node;
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
  struct fs_node node;
  bool mounted;
};

static struct mount mounts[512];
static struct linked_list process_list;
static size_t blocked_calls;

static int64_t mount_handler(__attribute__((unused)) uint64_t arg0, __attribute__((unused)) uint64_t arg1, __attribute__((unused)) uint64_t arg2, uint64_t address, uint64_t size) {
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
  char* buffer = (char*) address;
  if (buffer[size - 1]) {
    syslog(LOG_DEBUG, "Path isn't null terminated");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (buffer[0] != '/') {
    syslog(LOG_DEBUG, "Path isn't absolute");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (buffer[size - 2] != '/') {
    syslog(LOG_DEBUG, "Path isn't a directory");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  for (size_t i = 0; i < size - 1; i++) {
    if (!isalnum(buffer[i]) && buffer[i] != '/') {
      syslog(LOG_DEBUG, "Path contains invalid characters");
      return -IPC_ERR_INVALID_ARGUMENTS;
    }
  }
  for (size_t i = 0; i < 512; i++) {
    if (!mounts[i].path) {
      mounts[i].path = strdup(buffer);
      mounts[i].pid = pid;
      mounts[i].node.nfds = 1; // Prevent node from being freed
      return 0;
    }
  }
  syslog(LOG_CRIT, "Reached maximum number of mounts");
  return -IPC_ERR_PROGRAM_DEFINED;
}
static int64_t finish_mount_handler(uint64_t inode, __attribute__((unused)) uint64_t arg1, __attribute__((unused)) uint64_t arg2, uint64_t address, uint64_t size) {
  if (size != sizeof(struct vfs_stat)) {
    syslog(LOG_DEBUG, "Invalid stat size");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t pid = get_ipc_caller_pid();
  for (size_t i = 0; i < 512; i++) {
    if (mounts[i].pid == pid) {
      memcpy(&mounts[i].node.stat, (void*) address, sizeof(struct vfs_stat));
      if (mounts[i].node.stat.type != VFS_TYPE_DIR) {
        syslog(LOG_DEBUG, "Mount isn't a directory");
        return -IPC_ERR_INVALID_ARGUMENTS;
      }
      mounts[i].mounted = true;
      mounts[i].node.inode = inode;
      for (size_t j = 0; j < blocked_calls; j++) {
        ipc_unblock(0);
      }
      blocked_calls = 0;
      return 0;
    }
  }
  syslog(LOG_DEBUG, "Process doesn't handle mount");
  return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
}
static void free_node(struct fs_node* node) {
  struct fs_node* parent = node->parent;
  if (!node->nfds && !node->children.first) {
    linked_list_remove(&node->parent->children, &node->list_member);
    free(node);
  }
  if (parent) {
    free_node(parent);
  }
}
static int64_t open_file_handler(uint64_t flags, __attribute__((unused)) uint64_t arg1, __attribute__((unused)) uint64_t arg2, uint64_t address, uint64_t size) {
  size_t access_flags = flags & (O_RDONLY | O_RDWR | O_WRONLY);
  if (access_flags != O_RDONLY && access_flags != O_RDWR && access_flags != O_WRONLY) {
    syslog(LOG_DEBUG, "Invalid flags");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if ((flags & O_RDONLY) && (flags & O_TRUNC)) {
    syslog(LOG_DEBUG, "Invalid flags");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  char* buffer = (char*) address;
  if (buffer[size - 1]) {
    syslog(LOG_DEBUG, "Path isn't null terminated");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (buffer[0] != '/') {
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  for (size_t i = 0; i < size - 1; i++) {
    if (!isprint(buffer[i])) {
      return -IPC_ERR_INVALID_ARGUMENTS;
    }
  }
  int mount_i = -1;
  size_t mount_size = 0;
  for (size_t i = 0; i < 512; i++) {
    if (mounts[i].path && strlen(mounts[i].path) > mount_size && !strncmp(mounts[i].path, buffer, strlen(mounts[i].path))) {
      mount_i = i;
      mount_size = strlen(mounts[i].path);
    }
  }
  if (mount_i == -1 || !mounts[mount_i].mounted) {
    blocked_calls++;
    return -IPC_ERR_BLOCK;
  }
  struct fs_node* node = &mounts[mount_i].node;
  char* next_part;
  for (char* part = strtok(buffer + mount_size, "/"); part; part = next_part) {
    if (node->stat.type != VFS_TYPE_DIR) {
      free_node(node);
      return -IPC_ERR_INVALID_ARGUMENTS;
    }
    if (strlen(part) >= 256) {
      free_node(node);
      return -IPC_ERR_INVALID_ARGUMENTS;
    }
    bool found_child = false;
    for (struct fs_node* child = (struct fs_node*) node->children.first; child; child = (struct fs_node*) child->list_member.next) {
      if (!strcmp(child->name, part)) {
        node = child;
        found_child = true;
        break;
      }
    }
    next_part = strtok(0, "/");
    if (found_child) {
      if (!next_part) {
        send_pid_ipc_call(mounts[mount_i].pid, IPC_VFSD_FS_OPEN, flags, node->inode, 0, (uintptr_t) part, strlen(part) + 1);
      }
      continue;
    }
    long inode;
    if (next_part) {
      inode = send_pid_ipc_call(mounts[mount_i].pid, IPC_VFSD_FS_OPEN, O_RDONLY, node->inode, 0, (uintptr_t) part, strlen(part) + 1);
    } else {
      inode = send_pid_ipc_call(mounts[mount_i].pid, IPC_VFSD_FS_OPEN, flags, node->inode, 0, (uintptr_t) part, strlen(part) + 1);
    }
    if (inode < 0) {
      free_node(node);
      return -IPC_ERR_PROGRAM_DEFINED;
    }
    struct fs_node* child = calloc(1, sizeof(struct fs_node));
    child->parent = node;
    linked_list_insert(&node->children, &child->list_member, child);
    strcpy(child->name, part);
    child->inode = inode;
    send_pid_ipc_call(mounts[mount_i].pid, IPC_VFSD_FS_STAT, child->inode, 0, 0, (uintptr_t) &child->stat, sizeof(struct vfs_stat));
    node = child;
  }
  pid_t caller_pid = get_ipc_caller_pid();
  struct process* process = 0;
  for (struct linked_list_member* member = process_list.first; member; member = member->next) {
    struct process* loop_process = member->node;
    if (loop_process->pid == caller_pid) {
      process = loop_process;
      break;
    }
  }
  if (!process) {
    process = calloc(1, sizeof(struct process) + 128 * sizeof(struct fd));
    process->pid = caller_pid;
    process->nfds = 128;
    linked_list_insert(&process_list, &process->list_member, process);
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
  fd->exists = true;
  fd->flags = flags;
  fd->node = node;
  fd->node->nfds++;
  return fd_i;
}
static int64_t close_file_handler(uint64_t fd_num, __attribute__((unused)) uint64_t arg1, __attribute__((unused)) uint64_t arg2, __attribute__((unused)) uint64_t arg3, __attribute__((unused)) uint64_t arg4) {
  pid_t caller_pid = get_ipc_caller_pid();
  for (struct linked_list_member* member = process_list.first; member; member = member->next) {
    struct process* process = member->node;
    if (process->pid == caller_pid) {
      if (fd_num >= process->nfds || !process->fds[fd_num].exists) {
        syslog(LOG_DEBUG, "File descriptor doesn't exist");
        return -IPC_ERR_INVALID_ARGUMENTS;
      }
      struct fs_node* node = process->fds[fd_num].node;
      node->nfds--;
      free_node(node);
      memset(&process->fds[fd_num], 0, sizeof(struct fd));
      return 0;
    }
  }
  syslog(LOG_DEBUG, "File descriptor doesn't exist");
  return -IPC_ERR_INVALID_ARGUMENTS;
}
static int64_t handle_transfer(uint64_t fd_num, __attribute__((unused)) uint64_t arg1, __attribute__((unused)) uint64_t arg2, uint64_t address, uint64_t size, bool write) {
  pid_t caller_pid = get_ipc_caller_pid();
  for (struct linked_list_member* member = process_list.first; member; member = member->next) {
    struct process* process = member->node;
    if (process->pid == caller_pid) {
      if (fd_num >= process->nfds || !process->fds[fd_num].exists) {
        syslog(LOG_DEBUG, "File descriptor doesn't exist");
        return -IPC_ERR_INVALID_ARGUMENTS;
      }
      if (process->fds[fd_num].node->stat.type != VFS_TYPE_FILE) {
        return -IPC_ERR_PROGRAM_DEFINED;
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
      struct fs_node* parent = process->fds[fd_num].node;
      while (parent->parent) {
        parent = parent->parent;
      }
      pid_t pid;
      for (size_t i = 0;; i++) {
        if (&mounts[i].node == parent) {
          pid = mounts[i].pid;
          break;
        }
      }
      int64_t return_value = send_pid_ipc_call(pid, call, process->fds[fd_num].node->inode, process->fds[fd_num].position, 0, address, size);
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
  return handle_transfer(fd_num, arg1, arg2, address, size, false);
}
static int64_t write_file_handler(uint64_t fd_num, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size) {
  return handle_transfer(fd_num, arg1, arg2, address, size, true);
}
static int64_t seek_file_handler(uint64_t fd_num, uint64_t mode, uint64_t arg_position, __attribute__((unused)) uint64_t arg3, __attribute__((unused)) uint64_t arg4) {
  if (mode >= 2) {
    syslog(LOG_DEBUG, "Argument out of range");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t caller_pid = get_ipc_caller_pid();
  for (struct linked_list_member* member = process_list.first; member; member = member->next) {
    struct process* process = member->node;
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
static int64_t clone_fds_handler(uint64_t fork, __attribute__((unused)) uint64_t arg1, __attribute__((unused)) uint64_t arg2, __attribute__((unused)) uint64_t arg3, __attribute__((unused)) uint64_t arg4) {
  if (fork >= 2) {
    syslog(LOG_DEBUG, "Argument out of range");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t spawned_pid = get_caller_spawned_pid();
  if (!spawned_pid) {
    syslog(LOG_DEBUG, "Not currently spawning a process");
    return -IPC_ERR_PROGRAM_DEFINED;
  }
  pid_t pid = get_ipc_caller_pid();
  struct process* cloned_process = 0;
  for (struct linked_list_member* member = process_list.first; member; member = member->next) {
    struct process* process = member->node;
    if (process->pid == pid) {
      cloned_process = process;
      break;
    }
  }
  if (!cloned_process) {
    return 0;
  }
  struct process* spawned_process = 0;
  for (struct linked_list_member* member = process_list.first; member; member = member->next) {
    struct process* process = member->node;
    if (process->pid == spawned_pid) {
      spawned_process = process;
      break;
    }
  }
  if (spawned_process) {
    syslog(LOG_DEBUG, "Process already exists");
    return -IPC_ERR_PROGRAM_DEFINED;
  }
  size_t nfds = 128;
  if (fork) {
    nfds = cloned_process->nfds;
  }
  spawned_process = calloc(1, sizeof(struct process) + nfds * sizeof(struct fd));
  spawned_process->pid = spawned_pid;
  spawned_process->nfds = nfds;
  linked_list_insert(&process_list, &spawned_process->list_member, spawned_process);
  size_t clone_until = 3;
  if (fork) {
    clone_until = nfds;
  }
  for (size_t i = 0; i < clone_until; i++) {
    memcpy(&spawned_process->fds[i], &cloned_process->fds[i], sizeof(struct fd));
    if (spawned_process->fds[i].exists) {
      spawned_process->fds[i].node->nfds++;
    }
  }
  return 0;
}
int main(void) {
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  register_ipc();
  listen_exits();
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_LISTEN_EXITS);
  register_ipc_call(IPC_VFSD_CLOSE, close_file_handler, 1);
  register_ipc_call(IPC_VFSD_SEEK, seek_file_handler, 3);
  register_ipc_call(IPC_VFSD_CLONE_FDS, clone_fds_handler, 1);
  register_ipc_call(IPC_VFSD_OPEN, open_file_handler, 1);
  register_ipc_call(IPC_VFSD_MOUNT, mount_handler, 0);
  register_ipc_call(IPC_VFSD_WRITE, write_file_handler, 1);
  register_ipc_call(IPC_VFSD_FINISH_MOUNT, finish_mount_handler, 1);
  register_ipc_call(IPC_VFSD_READ, read_file_handler, 1);
  while (true) {
    handle_ipc();
    pid_t pid;
    while ((pid = get_exited_pid())) {
      struct linked_list_member* next_member;
      for (struct linked_list_member* member = process_list.first; member; member = next_member) {
        next_member = member->next;
        struct process* process = member->node;
        if (process->pid == pid) {
          for (size_t i = 0; i < process->nfds; i++) {
            if (process->fds[i].exists) {
              process->fds[i].node->nfds--;
              free_node(process->fds[i].node);
            }
          }
          linked_list_remove(&process_list, &process->list_member);
          free(process);
        }
      }
    }
  }
}
