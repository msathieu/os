#include <capability.h>
#include <ipccalls.h>
#include <spawn.h>
#include <stdlib.h>
#include <syslog.h>
#include <unistd.h>
#define LVM_MAGIC 0x10f2af8ac29eb55
#define SVFS_MAGIC 0x55ce581be6Bfa5a6
#define TMP_MAGIC 0x5381835340924865
#define LVM_SECTOR (1024 * 1024)

struct lvm_volume {
  uint64_t filesystem;
  uint64_t size;
  uint32_t sectors[1024];
};
static struct lvm_header {
  uint64_t magic;
  uint32_t version;
  uint32_t volumes[1024];
} header;

static pid_t parent_pid;
static pid_t volume_pids[1024];
static struct lvm_volume* volumes[1024];

static int64_t handle_transfer(uint64_t offset, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size, bool write) {
  if (arg1 || arg2) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t caller_pid = get_ipc_caller_pid();
  int volume_i = -1;
  for (size_t i = 0; i < 1024; i++) {
    if (caller_pid == volume_pids[i]) {
      volume_i = i;
      break;
    }
  }
  if (volume_i == -1) {
    syslog(LOG_DEBUG, "No permission to access disk");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  if (offset >= volumes[volume_i]->size) {
    return 0;
  }
  if (size > volumes[volume_i]->size - offset) {
    size = volumes[volume_i]->size - offset;
  }
  size_t total_size = offset + size;
  size_t written_size = 0;
  for (size_t i = offset / LVM_SECTOR * LVM_SECTOR; i < total_size; i += LVM_SECTOR) {
    size_t offset_i = 0;
    if (i == offset / LVM_SECTOR * LVM_SECTOR) {
      offset_i = offset % LVM_SECTOR;
    }
    size_t size_i = LVM_SECTOR;
    if (total_size - i < LVM_SECTOR) {
      size_i = total_size - i;
    }
    if (offset_i) {
      size_i -= offset_i;
    }
    uint8_t call = IPC_VFSD_FS_READ;
    if (write) {
      call = IPC_VFSD_FS_WRITE;
    }
    written_size += send_pid_ipc_call(parent_pid, call, volumes[volume_i]->sectors[i / LVM_SECTOR] * LVM_SECTOR + offset_i, 0, 0, address, size_i);
    address += size_i;
  }
  return written_size;
}
static int64_t read_handler(uint64_t offset, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size) {
  return handle_transfer(offset, arg1, arg2, address, size, 0);
}
static int64_t write_handler(uint64_t offset, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size) {
  return handle_transfer(offset, arg1, arg2, address, size, 1);
}
int main(void) {
  register_ipc(1);
  parent_pid = getppid();
  send_pid_ipc_call(parent_pid, IPC_VFSD_FS_READ, 0, 0, 0, (uintptr_t) &header, sizeof(struct lvm_header));
  if (header.magic != LVM_MAGIC || header.version) {
    syslog(LOG_ERR, "Filesystem has invalid magic value or unknown version");
    return 1;
  }
  for (size_t i = 0; header.volumes[i]; i++) {
    volumes[i] = calloc(1, sizeof(struct lvm_volume));
    send_pid_ipc_call(parent_pid, IPC_VFSD_FS_READ, header.volumes[i] * LVM_SECTOR, 0, 0, (uintptr_t) volumes[i], sizeof(struct lvm_volume));
    if (volumes[i]->size > LVM_SECTOR * 1024) {
      syslog(LOG_ERR, "Filesystem has invalid size");
      return 1;
    }
    switch (volumes[i]->filesystem) {
    case SVFS_MAGIC:
      volume_pids[i] = spawn_process_raw("svfsd");
      send_ipc_call("vfsd", IPC_VFSD_MOUNT, 0, 0, 0, (uintptr_t) "/", 2);
      break;
    case TMP_MAGIC:
      volume_pids[i] = spawn_process("/sbin/tmpd");
      send_ipc_call("vfsd", IPC_VFSD_MOUNT, 0, 0, 0, (uintptr_t) "/tmp/", 6);
      break;
    default:
      continue;
    }
    grant_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
    start_process();
  }
  ipc_handlers[IPC_VFSD_FS_WRITE] = write_handler;
  ipc_handlers[IPC_VFSD_FS_READ] = read_handler;
  while (1) {
    handle_ipc();
  }
}
