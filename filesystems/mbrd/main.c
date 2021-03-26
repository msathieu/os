#include <capability.h>
#include <ipccalls.h>
#include <spawn.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#define MBR_TYPE_LVM 0xb9

struct mbr_partition {
  uint8_t attributes;
  uint8_t chs_start[3];
  uint8_t type;
  uint8_t chs_end[3];
  uint32_t lba_start;
  uint32_t num_sectors;
};
static struct mbr {
  uint8_t bootstrap[440];
  uint32_t disk_id;
  uint16_t reserved;
  struct mbr_partition partitions[4];
} __attribute__((packed)) mbr;

static pid_t parent_pid;
static pid_t child_pid[4];

static int64_t handle_transfer(uint64_t offset, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size, bool write) {
  if (arg1 || arg2) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t caller_pid = get_ipc_caller_pid();
  int partition_i = -1;
  for (size_t i = 0; i < 4; i++) {
    if (child_pid[i] == caller_pid) {
      partition_i = i;
      break;
    }
  }
  if (partition_i == -1) {
    syslog(LOG_DEBUG, "No permission to access disk");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  size_t total_size;
  if (__builtin_uaddl_overflow(offset, size, &total_size)) {
    syslog(LOG_DEBUG, "Can't access this much data");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (total_size > mbr.partitions[partition_i].num_sectors * 512) {
    syslog(LOG_DEBUG, "Can't access this much data");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  uint8_t call = IPC_VFSD_FS_READ;
  if (write) {
    call = IPC_VFSD_FS_WRITE;
  }
  send_pid_ipc_call(parent_pid, call, mbr.partitions[partition_i].lba_start * 512 + offset, 0, 0, address, size);
  return 0;
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
  send_pid_ipc_call(parent_pid, IPC_VFSD_FS_READ, 0, 0, 0, (uintptr_t) &mbr, sizeof(struct mbr));
  for (size_t i = 0; i < 4; i++) {
    switch (mbr.partitions[i].type) {
    case MBR_TYPE_LVM:
      child_pid[i] = spawn_process_raw("lvmd");
      grant_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
      grant_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_VFSD_MOUNT);
      start_process();
      break;
    }
  }
  ipc_handlers[IPC_VFSD_FS_WRITE] = write_handler;
  ipc_handlers[IPC_VFSD_FS_READ] = read_handler;
  while (1) {
    handle_ipc();
  }
}
