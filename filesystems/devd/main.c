#include <capability.h>
#include <ctype.h>
#include <ipccalls.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <vfs.h>

struct device {
  const char* name;
  pid_t pid;
};

static struct device devices[512];

static int64_t register_device_handler(__attribute__((unused)) uint64_t arg0, __attribute__((unused)) uint64_t arg1, __attribute__((unused)) uint64_t arg2, uint64_t address, uint64_t size) {
  if (!has_ipc_caller_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_VFSD_MOUNT)) {
    syslog(LOG_DEBUG, "No permission to mount filesystem");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  if (size == 1) {
    syslog(LOG_DEBUG, "No device name specified");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t pid = get_caller_spawned_pid();
  if (!pid) {
    syslog(LOG_DEBUG, "Not currently spawning a process");
    return -IPC_ERR_PROGRAM_DEFINED;
  }
  char* buffer = (char*) address;
  if (buffer[size - 1]) {
    syslog(LOG_DEBUG, "Name isn't null terminated");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  for (size_t i = 0; i < size - 1; i++) {
    if (!isalnum(buffer[i])) {
      syslog(LOG_DEBUG, "Name contains invalid characters");
      return -IPC_ERR_INVALID_ARGUMENTS;
    }
  }
  for (size_t i = 0; i < 512; i++) {
    if (!devices[i].name) {
      devices[i].name = strdup(buffer);
      devices[i].pid = pid;
      return 0;
    }
  }
  syslog(LOG_CRIT, "Reached maximum number of devices");
  return -IPC_ERR_PROGRAM_DEFINED;
}
static int64_t open_handler(__attribute__((unused)) uint64_t flags, __attribute__((unused)) uint64_t parent_inode, uint64_t arg2, uint64_t address, __attribute__((unused)) uint64_t size) {
  if (arg2) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (!has_ipc_caller_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_VFSD)) {
    syslog(LOG_DEBUG, "Not allowed to open file");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  char* buffer = (char*) address;
  for (size_t i = 0; i < 512; i++) {
    if (devices[i].name && !strcmp(devices[i].name, buffer)) {
      return i;
    }
  }
  return -IPC_ERR_INVALID_ARGUMENTS;
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
static int64_t handle_transfer(uint64_t inode, uint64_t offset, __attribute__((unused)) uint64_t arg2, uint64_t address, uint64_t size, bool write) {
  if (!has_ipc_caller_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_VFSD)) {
    syslog(LOG_DEBUG, "Not allowed to access file");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  uint8_t call = IPC_VFSD_FS_READ;
  if (write) {
    call = IPC_VFSD_FS_WRITE;
  }
  return send_pid_ipc_call(devices[inode].pid, call, offset, 0, 0, address, size);
}
static int64_t read_handler(uint64_t inode, uint64_t offset, uint64_t arg2, uint64_t address, uint64_t size) {
  return handle_transfer(inode, offset, arg2, address, size, false);
}
static int64_t write_handler(uint64_t inode, uint64_t offset, uint64_t arg2, uint64_t address, uint64_t size) {
  return handle_transfer(inode, offset, arg2, address, size, true);
}
int main(void) {
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  register_ipc();
  struct vfs_stat stat = {0};
  stat.type = VFS_TYPE_DIR;
  send_ipc_call("vfsd", IPC_VFSD_FINISH_MOUNT, 512, 0, 0, (uintptr_t) &stat, sizeof(struct vfs_stat));
  register_ipc_call(IPC_VFSD_FS_OPEN, open_handler, 2);
  register_ipc_call(IPC_VFSD_FS_WRITE, write_handler, 2);
  register_ipc_call(IPC_DEVD_REGISTER, register_device_handler, 0);
  register_ipc_call(IPC_VFSD_FS_READ, read_handler, 2);
  register_ipc_call(IPC_VFSD_FS_STAT, stat_handler, 1);
  while (true) {
    handle_ipc();
  }
}
