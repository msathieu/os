#include <capability.h>
#include <ipc.h>
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

static int64_t read_handler(uint64_t offset, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size) {
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
  size_t total_size;
  if (__builtin_uaddl_overflow(offset, size, &total_size)) {
    syslog(LOG_DEBUG, "Can't access this much data");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (total_size > volumes[volume_i]->size || total_size > LVM_SECTOR * 1024) {
    syslog(LOG_DEBUG, "Can't access this much data");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
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
    send_pid_ipc_call(parent_pid, IPC_CALL_MEMORY_SHARING_RW, volumes[volume_i]->sectors[i / LVM_SECTOR] * LVM_SECTOR + offset_i, 0, 0, address, size_i);
    address += size_i;
  }
  return 0;
}
int main(void) {
  register_ipc(1);
  parent_pid = getppid();
  send_pid_ipc_call(parent_pid, IPC_CALL_MEMORY_SHARING_RW, 0, 0, 0, (uintptr_t) &header, sizeof(struct lvm_header));
  if (header.magic != LVM_MAGIC || header.version) {
    syslog(LOG_ERR, "Filesystem has invalid magic value or unknown version");
    return 1;
  }
  for (size_t i = 0; header.volumes[i]; i++) {
    volumes[i] = malloc(sizeof(struct lvm_volume));
    send_pid_ipc_call(parent_pid, IPC_CALL_MEMORY_SHARING_RW, header.volumes[i] * LVM_SECTOR, 0, 0, (uintptr_t) volumes[i], sizeof(struct lvm_volume));
    switch (volumes[i]->filesystem) {
    case SVFS_MAGIC:
      volume_pids[i] = spawn_process_raw("svfsd");
      send_ipc_call("vfsd", 0, 0, 0, 0, 0, 0);
      break;
    case TMP_MAGIC:
      volume_pids[i] = spawn_process("/sbin/tmpd");
      break;
    }
    grant_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
    start_process();
  }
  ipc_handlers[IPC_CALL_MEMORY_SHARING_RW] = read_handler;
  while (1) {
    handle_ipc();
  }
}
