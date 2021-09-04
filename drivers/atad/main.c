#include <ata/ata.h>
#include <capability.h>
#include <ioports.h>
#include <ipccalls.h>
#include <irq.h>
#include <spawn.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

static uint16_t base_ports[] = {0x1f0, 0x170};
static uint16_t alternate_ports[] = {0x3f6, 0x376};
static pid_t child_pid[4];
static bool is_atapi[4];

static void identify(int bus, int drive) {
  if (inb(alternate_ports[bus]) == 0xff) {
    return;
  }
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
  size_t tries = 0;
  while (inb(alternate_ports[bus]) & ATA_STATUS_BSY) {
    tries++;
    if (tries == 100) {
      syslog(LOG_ERR, "ATA retry limit exceeded");
      exit(1);
    }
  }
  if (inb(base_ports[bus] + ATA_PORT_LBAMIDDLE) || inb(base_ports[bus] + ATA_PORT_LBAHIGH)) {
    is_atapi[bus * 2 + drive] = 1;
    outb(base_ports[bus] + ATA_PORT_COMMAND, ATAPI_CMD_IDENTIFY);
    tries = 0;
    while (inb(alternate_ports[bus]) & ATA_STATUS_BSY) {
      tries++;
      if (tries == 100) {
        syslog(LOG_ERR, "ATA retry limit exceeded");
        exit(1);
      }
    }
  }
  tries = 0;
  while (1) {
    uint8_t status = inb(base_ports[bus] + ATA_PORT_COMMAND);
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
    tries++;
    if (tries == 100) {
      syslog(LOG_ERR, "ATA retry limit exceeded");
      exit(1);
    }
  }
}
static void prepare_transfer(int bus, int drive, size_t lba) {
  if (is_atapi[bus * 2 + drive]) {
    outb(base_ports[bus] + ATA_PORT_DRIVE, drive << 4);
  } else {
    outb(base_ports[bus] + ATA_PORT_DRIVE, ATA_DRIVE_PIO | drive << 4 | ((lba >> 24) & 0xf));
  }
  if (is_atapi[bus * 2 + drive]) {
    outb(base_ports[bus] + ATA_PORT_FEATURES, 0);
    outb(base_ports[bus] + ATA_PORT_LBAMIDDLE, (uint8_t) ATAPI_SECTOR);
    outb(base_ports[bus] + ATA_PORT_LBAHIGH, ATAPI_SECTOR >> 8);
    outb(base_ports[bus] + ATA_PORT_COMMAND, ATAPI_CMD_PACKET);
    size_t tries = 0;
    while (inb(alternate_ports[bus]) & ATA_STATUS_BSY) {
      tries++;
      if (tries == 100) {
        syslog(LOG_ERR, "ATA retry limit exceeded");
        exit(1);
      }
    }
    tries = 0;
    while (!(inb(alternate_ports[bus]) & ATA_STATUS_DRQ)) {
      tries++;
      if (tries == 100) {
        syslog(LOG_ERR, "ATA retry limit exceeded");
        exit(1);
      }
    }
  } else {
    outb(base_ports[bus] + ATA_PORT_SECTOR_COUNT, 1);
    outb(base_ports[bus] + ATA_PORT_LBALOW, lba);
    outb(base_ports[bus] + ATA_PORT_LBAMIDDLE, lba >> 8);
    outb(base_ports[bus] + ATA_PORT_LBAHIGH, lba >> 16);
  }
}
static void read(int bus, int drive, size_t lba, uint8_t* buffer) {
  prepare_transfer(bus, drive, lba);
  if (is_atapi[bus * 2 + drive]) {
    uint8_t cmd[12] = {ATAPI_CMD_READ, 0, lba >> 24, lba >> 16, lba >> 8, lba, 0, 0, 0, 1, 0, 0};
    for (size_t i = 0; i < 6; i++) {
      outw(base_ports[bus], ((uint16_t*) cmd)[i]);
    }
  } else {
    outb(base_ports[bus] + ATA_PORT_COMMAND, ATA_CMD_READ);
  }
  wait_irq();
  size_t sector = ATA_SECTOR;
  if (is_atapi[bus * 2 + drive]) {
    sector = ATAPI_SECTOR;
  }
  for (size_t i = 0; i < sector / 2; i++) {
    uint16_t value = inw(base_ports[bus]);
    buffer[i * 2] = value;
    buffer[i * 2 + 1] = value >> 8;
  }
  if (is_atapi[bus * 2 + drive]) {
    size_t tries = 0;
    while (inb(alternate_ports[bus]) & (ATA_STATUS_BSY | ATA_STATUS_DRQ)) {
      tries++;
      if (tries == 100) {
        syslog(LOG_ERR, "ATA retry limit exceeded");
        exit(1);
      }
    }
  }
}
static void write(int bus, int drive, size_t lba, uint8_t* buffer) {
  prepare_transfer(bus, drive, lba);
  outb(base_ports[bus] + ATA_PORT_COMMAND, ATA_CMD_WRITE);
  wait_irq();
  for (size_t i = 0; i < 256; i++) {
    uint16_t value = buffer[i * 2];
    value |= buffer[i * 2 + 1] << 8;
    outw(base_ports[bus], value);
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
  size_t sector = ATA_SECTOR;
  if (is_atapi[bus * 2 + drive]) {
    sector = ATAPI_SECTOR;
  }
  uint8_t buffer[sector];
  read(bus, drive, offset / sector, buffer);
  size_t initial_size = sector - offset % sector;
  if (initial_size > size) {
    initial_size = size;
  }
  memcpy((void*) address, (void*) buffer + offset % sector, initial_size);
  address += initial_size;
  size_t total_size;
  if (__builtin_uaddl_overflow(offset, size, &total_size)) {
    syslog(LOG_DEBUG, "Can't access this much data");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  size_t aligned_size;
  if (__builtin_uaddl_overflow(total_size, sector - 1, &aligned_size)) {
    syslog(LOG_DEBUG, "Can't access this much data");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  for (size_t i = offset / sector + 1; i < aligned_size / sector; i++) {
    if (total_size - i * sector >= sector) {
      read(bus, drive, i, (uint8_t*) address);
      address += sector;
    } else {
      read(bus, drive, i, buffer);
      size_t copy_size = total_size - i * sector;
      memcpy((void*) address, (void*) buffer, copy_size);
    }
  }
  return size;
}
static int64_t write_handler(uint64_t offset, uint64_t arg1, uint64_t arg2, uint64_t address, uint64_t size) {
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
  if (is_atapi[bus * 2 + drive]) {
    syslog(LOG_DEBUG, "Can't write to ATAPI drive");
    return -IPC_ERR_PROGRAM_DEFINED;
  }
  uint8_t buffer[ATA_SECTOR];
  read(bus, drive, offset / ATA_SECTOR, buffer);
  size_t initial_size = ATA_SECTOR - offset % ATA_SECTOR;
  if (initial_size > size) {
    initial_size = size;
  }
  memcpy((void*) buffer + offset % ATA_SECTOR, (void*) address, initial_size);
  write(bus, drive, offset / ATA_SECTOR, buffer);
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
  for (size_t i = offset / ATA_SECTOR + 1; i < aligned_size / ATA_SECTOR; i++) {
    if (total_size - i * ATA_SECTOR >= ATA_SECTOR) {
      write(bus, drive, i, (uint8_t*) address);
      address += ATA_SECTOR;
    } else {
      read(bus, drive, i, buffer);
      size_t copy_size = total_size - i * ATA_SECTOR;
      memcpy((void*) buffer, (void*) address, copy_size);
      write(bus, drive, i, buffer);
    }
  }
  return size;
}
int main(void) {
  register_ipc(1);
  identify(0, 0);
  identify(0, 1);
  identify(1, 0);
  identify(1, 1);
  clear_irqs();
  ipc_handlers[IPC_VFSD_FS_WRITE] = write_handler;
  ipc_handlers[IPC_VFSD_FS_READ] = read_handler;
  while (1) {
    handle_ipc();
  }
}
