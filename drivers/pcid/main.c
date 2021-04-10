#include <capability.h>
#include <ioports.h>
#include <ipccalls.h>
#include <syslog.h>
#define PCI_ADDRESS 0xcf8
#define PCI_DATA 0xcfc
#define PCI_VENDOR_ID 0
#define PCI_HEADER_TYPE 0xe

static uint32_t pci_readl(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
  outl(PCI_ADDRESS, 0x80000000 | bus << 16 | device << 11 | function << 8 | (offset & 0xfc));
  return inl(PCI_DATA);
}
static void pci_writel(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value) {
  outl(PCI_ADDRESS, 0x80000000 | bus << 16 | device << 11 | function << 8 | (offset & 0xfc));
  outl(PCI_DATA, value);
}
static uint16_t pci_readw(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
  return pci_readl(bus, device, function, offset) >> ((offset & 2) * 8);
}
static void pci_writew(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint16_t value) {
  uint32_t current_value = pci_readl(bus, device, function, offset);
  current_value &= ~(0xffff << ((offset & 2) * 8));
  current_value |= value << ((offset & 2) * 8);
  pci_writel(bus, device, function, offset, current_value);
}
static uint8_t pci_readb(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
  return pci_readl(bus, device, function, offset) >> ((offset & 3) * 8);
}
static void pci_writeb(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint8_t value) {
  uint32_t current_value = pci_readl(bus, device, function, offset);
  current_value &= ~(0xff << ((offset & 3) * 8));
  current_value |= value << ((offset & 3) * 8);
  pci_writel(bus, device, function, offset, current_value);
}
static int64_t access_handler(uint64_t write, uint64_t width, uint64_t address, uint64_t value, uint64_t arg4) {
  if (arg4) {
    syslog(LOG_DEBUG, "Reserved argument is set");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  if (!has_ipc_caller_capability(CAP_NAMESPACE_DRIVERS, CAP_PCID_ACCESS)) {
    syslog(LOG_DEBUG, "No permission to access PCI");
    return -IPC_ERR_INSUFFICIENT_PRIVILEGE;
  }
  switch (write) {
  case 0:
    switch (width) {
    case 0:
      return pci_readb(address >> 24, address >> 16, address >> 8, address);
    case 1:
      return pci_readw(address >> 24, address >> 16, address >> 8, address);
    case 2:
      return pci_readl(address >> 24, address >> 16, address >> 8, address);
    default:
      syslog(LOG_DEBUG, "Argument out of range");
      return -IPC_ERR_INVALID_ARGUMENTS;
    }
  case 1:
    switch (width) {
    case 0:
      pci_writeb(address >> 24, address >> 16, address >> 8, address, value);
      break;
    case 1:
      pci_writew(address >> 24, address >> 16, address >> 8, address, value);
      break;
    case 2:
      pci_writel(address >> 24, address >> 16, address >> 8, address, value);
      break;
    default:
      syslog(LOG_DEBUG, "Argument out of range");
      return -IPC_ERR_INVALID_ARGUMENTS;
    }
    break;
  default:
    syslog(LOG_DEBUG, "Argument out of range");
    return -IPC_ERR_INVALID_ARGUMENTS;
  }
  return 0;
}
static void load_device(__attribute__((unused)) uint8_t bus, __attribute__((unused)) uint8_t device, __attribute__((unused)) uint8_t function) {
}
int main(void) {
  register_ipc(0);
  for (size_t bus = 0; bus < 256; bus++) {
    for (size_t device = 0; device < 32; device++) {
      size_t nfunctions = 1;
      if (pci_readb(bus, device, 0, PCI_HEADER_TYPE) & 0x80) {
        nfunctions = 8;
      }
      for (size_t i = 0; i < nfunctions; i++) {
        uint16_t vendor = pci_readw(bus, device, i, PCI_VENDOR_ID);
        if (vendor != 0xffff) {
          load_device(bus, device, i);
        }
      }
    }
  }
  ipc_handlers[IPC_PCID_ACCESS] = access_handler;
  while (1) {
    handle_ipc();
  }
}
