#include <capability.h>
#include <fcntl.h>
#include <ipccalls.h>
#include <monocypher.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <vfs.h>
#define SVFS_MAGIC 0x55ce581be6Bfa5a6

struct svfs_file {
  uint8_t name[256];
  uint64_t offset;
  uint64_t size;
  uint8_t hash[64];
  uint8_t type;
};
struct svfs_header {
  uint8_t signature[64];
  uint64_t magic;
  uint32_t version;
  uint32_t nfiles;
  struct svfs_file files[];
};

static struct svfs_header* header;
static pid_t parent_pid;
extern int _binary____public_key_start;

static int64_t open_handler(uint64_t flags, uint64_t parent_inode, __attribute__((unused)) uint64_t arg2, uint64_t address, uint64_t size) {
  if (!has_ipc_caller_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_VFSD)) {
    syslog(LOG_DEBUG, "Not allowed to open file");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  if (flags & (O_RDWR | O_WRONLY)) {
    syslog(LOG_DEBUG, "Not allowed to write to file");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  size_t parent_path_size = strlen((char*) header->files[parent_inode].name);
  char* full_path = malloc(parent_path_size + 1 + size);
  strcpy(full_path, (char*) header->files[parent_inode].name);
  strcat(full_path, "/");
  memcpy(full_path + parent_path_size + 1, (void*) address, size);
  for (size_t i = 0; i < header->nfiles; i++) {
    if (parent_path_size + 1 + size != strlen((char*) header->files[i].name) + 1) {
      continue;
    }
    if (!strcmp((char*) header->files[i].name, (char*) full_path)) {
      free(full_path);
      return i;
    }
  }
  free(full_path);
  return -IPC_ERR_INVALID_ARGUMENTS;
}
static int64_t stat_handler(uint64_t inode, __attribute__((unused)) uint64_t arg1, __attribute__((unused)) uint64_t arg2, uint64_t address, __attribute__((unused)) uint64_t size) {
  if (!has_ipc_caller_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_VFSD)) {
    syslog(LOG_DEBUG, "Not allowed to stat file");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  struct vfs_stat* stat = (struct vfs_stat*) address;
  stat->type = header->files[inode].type;
  return 0;
}
static int64_t read_handler(uint64_t inode, uint64_t offset, __attribute__((unused)) uint64_t arg2, uint64_t address, uint64_t size) {
  if (!has_ipc_caller_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_VFSD)) {
    syslog(LOG_DEBUG, "Not allowed to read file");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  if (offset >= header->files[inode].size) {
    return 0;
  }
  if (size > header->files[inode].size - offset) {
    size = header->files[inode].size - offset;
  }
  return send_pid_ipc_call(parent_pid, IPC_VFSD_FS_READ, sizeof(struct svfs_header) + header->nfiles * sizeof(struct svfs_file) + header->files[inode].offset + offset, 0, 0, address, size);
}
int main(void) {
  drop_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
  register_ipc(1);
  parent_pid = getppid();
  header = calloc(1, sizeof(struct svfs_header));
  send_pid_ipc_call(parent_pid, IPC_VFSD_FS_READ, 0, 0, 0, (uintptr_t) header, sizeof(struct svfs_header));
  if (header->magic != SVFS_MAGIC || header->version) {
    syslog(LOG_ERR, "Filesystem has invalid magic value or unknown version");
    return 1;
  }
  if (header->nfiles > 4096) {
    return 1;
  }
  header = realloc(header, sizeof(struct svfs_header) + header->nfiles * sizeof(struct svfs_file));
  memset((void*) header + sizeof(struct svfs_header), 0, header->nfiles * sizeof(struct svfs_file));
  send_pid_ipc_call(parent_pid, IPC_VFSD_FS_READ, sizeof(struct svfs_header), 0, 0, (uintptr_t) header + sizeof(struct svfs_header), header->nfiles * sizeof(struct svfs_file));
  if (crypto_check(header->signature, (uint8_t*) &_binary____public_key_start, (uint8_t*) header + offsetof(struct svfs_header, magic), sizeof(struct svfs_header) - offsetof(struct svfs_header, magic) + header->nfiles * sizeof(struct svfs_file))) {
    syslog(LOG_ERR, "Filesystem has invalid signature");
    return 1;
  }
  for (size_t i = 0; i < header->nfiles; i++) {
    if (header->files[i].type != VFS_TYPE_FILE) {
      continue;
    }
    uint8_t* file = calloc(header->files[i].size, 1);
    send_pid_ipc_call(parent_pid, IPC_VFSD_FS_READ, sizeof(struct svfs_header) + header->nfiles * sizeof(struct svfs_file) + header->files[i].offset, 0, 0, (uintptr_t) file, header->files[i].size);
    uint8_t hash[64];
    crypto_blake2b(hash, file, header->files[i].size);
    free(file);
    if (memcmp(hash, header->files[i].hash, 64)) {
      syslog(LOG_ERR, "File %s has invalid hash", header->files[i].name);
      return 1;
    }
  }
  struct vfs_stat stat = {0};
  stat.type = VFS_TYPE_DIR;
  send_ipc_call("vfsd", IPC_VFSD_FINISH_MOUNT, 0, 0, 0, (uintptr_t) &stat, sizeof(struct vfs_stat));
  register_ipc_call(IPC_VFSD_FS_OPEN, open_handler, 2);
  register_ipc_call(IPC_VFSD_FS_READ, read_handler, 2);
  register_ipc_call(IPC_VFSD_FS_STAT, stat_handler, 1);
  while (1) {
    handle_ipc();
  }
}
