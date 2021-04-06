#include <__/syscall.h>
#include <ioports.h>
#include <ipccalls.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

void laihost_log(int priority, const char* msg) {
  syslog(priority, msg);
}
_Noreturn void laihost_panic(const char* msg) {
  syslog(LOG_CRIT, "PANIC: %s", msg);
  exit(1);
}
void* laihost_malloc(size_t size) {
  return malloc(size);
}
void* laihost_realloc(void* ptr, size_t size) {
  return realloc(ptr, size);
}
void laihost_free(void* ptr) {
  free(ptr);
}
void* laihost_map(uintptr_t address, size_t size) {
  uintptr_t end = (address + size + 0xfff) / 0x1000 * 0x1000;
  size_t offset = address % 0x1000;
  address = address / 0x1000 * 0x1000;
  return (void*) map_physical_memory(address, end - address, 0) + offset;
}
void laihost_unmap(__attribute__((unused)) void* ptr, __attribute__((unused)) size_t size) {
}
void* laihost_scan(char* signature, size_t index) {
  char os_signature[5];
  strncpy(os_signature, signature, 4);
  return (void*) _syscall(_SYSCALL_GET_ACPI_TABLE, os_signature[0], os_signature[1], os_signature[2], os_signature[3], index);
}
void laihost_outb(uint16_t port, uint8_t value) {
  outb(port, value);
}
void laihost_outw(uint16_t port, uint16_t value) {
  outw(port, value);
}
void laihost_outd(uint16_t port, uint32_t value) {
  outl(port, value);
}
uint8_t laihost_inb(uint16_t port) {
  return inb(port);
}
uint16_t laihost_inw(uint16_t port) {
  return inw(port);
}
uint32_t laihost_ind(uint16_t port) {
  return inl(port);
}
void laihost_pci_writeb(__attribute__((unused)) uint16_t segment, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset, uint8_t value) {
  send_ipc_call("pcid", IPC_PCID_ACCESS, 1, 0, offset | function << 8 | device << 16 | bus << 24, value, 0);
}
void laihost_pci_writew(__attribute__((unused)) uint16_t segment, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset, uint16_t value) {
  send_ipc_call("pcid", IPC_PCID_ACCESS, 1, 1, offset | function << 8 | device << 16 | bus << 24, value, 0);
}
void laihost_pci_writed(__attribute__((unused)) uint16_t segment, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset, uint32_t value) {
  send_ipc_call("pcid", IPC_PCID_ACCESS, 1, 2, offset | function << 8 | device << 16 | bus << 24, value, 0);
}
uint8_t laihost_pci_readb(__attribute__((unused)) uint16_t segment, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset) {
  return send_ipc_call("pcid", IPC_PCID_ACCESS, 0, 0, offset | function << 8 | device << 16 | bus << 24, 0, 0);
}
uint16_t laihost_pci_readw(__attribute__((unused)) uint16_t segment, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset) {
  return send_ipc_call("pcid", IPC_PCID_ACCESS, 0, 1, offset | function << 8 | device << 16 | bus << 24, 0, 0);
}
uint32_t laihost_pci_readd(__attribute__((unused)) uint16_t segment, uint8_t bus, uint8_t device, uint8_t function, uint16_t offset) {
  return send_ipc_call("pcid", IPC_PCID_ACCESS, 0, 2, offset | function << 8 | device << 16 | bus << 24, 0, 0);
}
void laihost_sleep(size_t duration) {
  _syscall(_SYSCALL_SLEEP, duration, 0, 0, 0, 0);
}
