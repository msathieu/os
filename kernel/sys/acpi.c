#include <cpu/paging.h>
#include <panic.h>
#include <stdlib.h>
#include <string.h>
#include <struct.h>
#include <sys/hpet.h>
#include <sys/madt.h>

static unsigned short nheaders;
static struct acpi_header** headers;

static void check_checksum(struct acpi_header* header) {
  uint8_t checksum = 0;
  for (size_t i = 0; i < header->size; i++) {
    checksum += ((uint8_t*) header)[i];
  }
  if (checksum) {
    panic("Invalid checksum");
  }
}
static struct acpi_header* find_table(const char* signature) {
  for (size_t i = 0; i < nheaders; i++) {
    if (!memcmp(headers[i]->signature, signature, 4)) {
      struct acpi_header* header = map_physical(convert_to_physical((uintptr_t) headers[i], current_pml4), headers[i]->size, 0, 0);
      check_checksum(header);
      return header;
    }
  }
  panic("Couldn't find ACPI table");
}
void parse_acpi(void) {
  if (loader_struct.rsdp.revision >= 2) {
    struct xsdt* xsdt = map_physical(loader_struct.rsdp.xsdt, sizeof(struct acpi_header), 0, 0);
    if (memcmp(xsdt->header.signature, "XSDT", 4)) {
      panic("Invalid XSDT signature");
    }
    xsdt = map_physical(loader_struct.rsdp.xsdt, xsdt->header.size, 0, 0);
    check_checksum(&xsdt->header);
    nheaders = (xsdt->header.size - sizeof(struct acpi_header)) / 8;
    headers = calloc(nheaders, 8);
    for (size_t i = 0; i < nheaders; i++) {
      headers[i] = map_physical(xsdt->tables[i], sizeof(struct acpi_header), 0, 0);
    }
  } else {
    struct rsdt* rsdt = map_physical(loader_struct.rsdp.rsdt, sizeof(struct acpi_header), 0, 0);
    if (memcmp(rsdt->header.signature, "RSDT", 4)) {
      panic("Invalid RSDT signature");
    }
    rsdt = map_physical(loader_struct.rsdp.rsdt, rsdt->header.size, 0, 0);
    check_checksum(&rsdt->header);
    nheaders = (rsdt->header.size - sizeof(struct acpi_header)) / 4;
    headers = calloc(nheaders, 8);
    for (size_t i = 0; i < nheaders; i++) {
      headers[i] = map_physical(rsdt->tables[i], sizeof(struct acpi_header), 0, 0);
    }
  }
  parse_madt(find_table("APIC"));
  setup_hpet(find_table("HPET"));
}
