#include <ata/ata.h>
#include <capability.h>
#include <ioports.h>
#include <ipccalls.h>
#include <irq.h>
#include <spawn.h>
#include <string.h>
#include <syslog.h>

static uint16_t base_ports[] = {0x1f0, 0x170};
static uint16_t alternate_ports[] = {0x3f6, 0x376};
static pid_t child_pid[4];

static void identify(int bus, int drive) {
  if (drive) {
    outb(base_ports[bus] + ATA_PORT_DRIVE, ATA_DRIVE_SLAVE);
  } else {
    outb(base_ports[bus] + ATA_PORT_DRIVE, ATA_DRIVE_MASTER);
  }
  outb(base_ports[bus] + ATA_PORT_SECTOR_COUNT, 0);
  outb(base_ports[bus] + ATA_PORT_LBALOW, 0);
  outb(base_ports[bus] + ATA_PORT_LBAMIDDLE, 0);
  outb(base_ports[bus] + ATA_PORT_LBAHIGH, 0);
  outb(base_ports[bus] + ATA_PORT_COMMAND, ATA_CMD_IDENTIFY);
  if (!inb(alternate_ports[bus])) {
    return;
  }
  while (inb(alternate_ports[bus]) & ATA_STATUS_BSY)
    ;
  if (inb(base_ports[bus] + ATA_PORT_LBAMIDDLE) || inb(base_ports[bus] + ATA_PORT_LBAHIGH)) {
    return;
  }
  while (1) {
    uint8_t status = inb(alternate_ports[bus]);
    if (status & ATA_STATUS_ERR) {
      syslog(LOG_ERR, "Error identifying drive %d on bus %d", drive, bus);
      return;
    }
    if (status & ATA_STATUS_DRQ) {
      for (size_t i = 0; i < 256; i++) {
        inw(base_ports[bus]);
      }
      child_pid[bus * 2 + drive] = spawn_process_raw("mbrd");
      grant_capability(CAP_NAMESPACE_KERNEL, CAP_KERNEL_PRIORITY);
      grant_capability(CAP_NAMESPACE_FILESYSTEMS, CAP_VFSD_MOUNT);
      start_process();
      return;
    }
  }
}
static void read(int bus, int drive, size_t lba, uint16_t* buffer) {
  if (drive) {
    outb(base_ports[bus] + ATA_PORT_DRIVE, ATA_DRIVE_SLAVE_PIO | ((lba >> 24) & 0xf));
  } else {
    outb(base_ports[bus] + ATA_PORT_DRIVE, ATA_DRIVE_MASTER_PIO | ((lba >> 24) & 0xf));
  }
  outb(base_ports[bus] + ATA_PORT_SECTOR_COUNT, 1);
  outb(base_ports[bus] + ATA_PORT_LBALOW, lba);
  outb(base_ports[bus] + ATA_PORT_LBAMIDDLE, lba >> 8);
  outb(base_ports[bus] + ATA_PORT_LBAHIGH, lba >> 16);
  outb(base_ports[bus] + ATA_PORT_COMMAND, ATA_CMD_READ);
  wait_irq();
  for (size_t i = 0; i < 256; i++) {
    buffer[i] = inw(base_ports[bus]);
  }
}
static int64_t read_handler(uint64_t offset, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size) {
  if (arg1 || arg2) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  pid_t caller_pid = get_ipc_caller_pid();
  if (caller_pid != child_pid[0] && caller_pid != child_pid[1] && caller_pid != child_pid[2] && caller_pid != child_pid[3]) {
    syslog(LOG_DEBUG, "No permission to access disk");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  int bus = 0, drive = 0;
  for (size_t i = 0; i < 4; i++) {
    if (child_pid[i] == caller_pid) {
      bus = i / 2;
      drive = i % 2;
      break;
    }
  }
  uint16_t buffer[256];
  read(bus, drive, offset / 512, buffer);
  size_t initial_size = 512 - offset % 512;
  if (initial_size > size) {
    initial_size = size;
  }
  memcpy((void*) address, (void*) buffer + offset % 512, initial_size);
  address += initial_size;
  size_t total_size;
  if (__builtin_uaddl_overflow(offset, size, &total_size)) {
    syslog(LOG_DEBUG, "Can't access this much data");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  size_t aligned_size;
  if (__builtin_uaddl_overflow(total_size, 511, &aligned_size)) {
    syslog(LOG_DEBUG, "Can't access this much data");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  for (size_t i = offset / 512 + 1; i < aligned_size / 512; i++) {
    if (total_size - i * 512 >= 512) {
      read(bus, drive, i, (uint16_t*) address);
      address += 512;
    } else {
      read(bus, drive, i, buffer);
      size_t copy_size = total_size - i * 512;
      memcpy((void*) address, (void*) buffer, copy_size);
    }
  }
  return 0;
}
int main(void) {
  register_ipc(1);
  identify(0, 0);
  identify(0, 1);
  identify(1, 0);
  identify(1, 1);
  clear_irqs();
  ipc_handlers[IPC_VFSD_FS_READ] = read_handler;
  while (1) {
    handle_ipc();
  }
}
