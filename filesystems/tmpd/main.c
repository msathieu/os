#include <capability.h>
#include <ctype.h>
#include <fcntl.h>
#include <ipccalls.h>
#include <linked_list.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <vfs.h>
#define TMP_MAGIC 0x5381835340924865

struct file {
  struct linked_list_member list_member;
  size_t inode;
  char* path;
  void* data;
  size_t size;
};

static struct linked_list files_list;
static size_t next_inode = 1;

static int64_t open_handler(uint64_t flags, __attribute__((unused)) uint64_t parent_inode, __attribute__((unused)) uint64_t arg2, uint64_t address, __attribute__((unused)) uint64_t size) {
  if (!has_ipc_caller_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_VFSD)) {
    syslog(LOG_DEBUG, "Not allowed to open file");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  char* buffer = (char*) address;
  for (struct file* file = (struct file*) files_list.first; file; file = (struct file*) file->list_member.next) {
    if (!strcmp(file->path, buffer)) {
      if (flags & O_TRUNC) {
        free(file->data);
        file->size = 0;
      }
      return file->inode;
    }
  }
  if (flags & O_CREAT) {
    struct file* file = calloc(1, sizeof(struct file));
    file->inode = next_inode++;
    file->path = strdup(buffer);
    insert_linked_list(&files_list, &file->list_member);
    return file->inode;
  } else {
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
}
static int64_t stat_handler(__attribute__((unused)) uint64_t inode, __attribute__((unused)) uint64_t arg1, __attribute__((unused)) uint64_t arg2, uint64_t address, __attribute__((unused)) uint64_t size) {
  if (!has_ipc_caller_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_VFSD)) {
    syslog(LOG_DEBUG, "Not allowed to stat file");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  struct vfs_stat* stat = (struct vfs_stat*) address;
  stat->type = VFS_TYPE_FILE;
  return 0;
}
static int64_t read_handler(uint64_t inode, uint64_t offset, __attribute__((unused)) uint64_t arg2, uint64_t address, uint64_t size) {
  if (!has_ipc_caller_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_VFSD)) {
    syslog(LOG_DEBUG, "Not allowed to read file");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  struct file* file;
  for (struct file* file_i = (struct file*) files_list.first;; file_i = (struct file*) file_i->list_member.next) {
    if (file_i->inode == inode) {
      file = file_i;
      break;
    }
  }
  if (offset >= file->size) {
    return 0;
  }
  if (size > file->size - offset) {
    size = file->size - offset;
  }
  memcpy((void*) address, file->data + offset, size);
  return size;
}
static int64_t write_handler(uint64_t inode, uint64_t offset, __attribute__((unused)) uint64_t arg2, uint64_t address, uint64_t size) {
  if (!has_ipc_caller_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_VFSD)) {
    syslog(LOG_DEBUG, "Not allowed to write to file");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  if (offset > (size_t) 1 << 32) {
    syslog(LOG_DEBUG, "Offset is too big");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  struct file* file;
  for (struct file* file_i = (struct file*) files_list.first;; file_i = (struct file*) file_i->list_member.next) {
    if (file_i->inode == inode) {
      file = file_i;
      break;
    }
  }
  size_t total_size = offset + size;
  if (total_size > file->size) {
    if (file->data) {
      file->data = realloc(file->data, total_size);
    } else {
      file->data = malloc(total_size);
    }
    memset(file->data + file->size, 0, total_size - file->size);
    file->size = total_size;
  }
  memcpy(file->data + offset, (void*) address, size);
  return size;
}
int main(void) {
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  register_ipc();
  struct vfs_stat stat = {0};
  stat.type = VFS_TYPE_DIR;
  send_ipc_call("vfsd", IPC_VFSD_FINISH_MOUNT, 0, 0, 0, (uintptr_t) &stat, sizeof(struct vfs_stat));
  register_ipc_call(IPC_VFSD_FS_OPEN, open_handler, 2);
  register_ipc_call(IPC_VFSD_FS_WRITE, write_handler, 2);
  register_ipc_call(IPC_VFSD_FS_READ, read_handler, 2);
  register_ipc_call(IPC_VFSD_FS_STAT, stat_handler, 1);
  while (true) {
    handle_ipc();
  }
}
