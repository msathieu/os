#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

struct acpi_rsdp {
  uint8_t signature[8];
  uint8_t checksum;
  uint8_t oem_id[6];
  uint8_t revision;
  uint32_t rsdt;
  // --- v2 ---
  uint32_t size;
  uint64_t xsdt;
  uint8_t extended_checksum;
  uint8_t reserved[3];
} __attribute__((packed));
struct acpi_header {
  uint8_t signature[4];
  uint32_t size;
  uint8_t revision;
  uint8_t checksum;
  uint8_t oem_id[6];
  uint8_t oem_table_id[8];
  uint32_t oem_revision;
  uint32_t creator_id;
  uint32_t creator_revision;
} __attribute__((packed));
struct rsdt {
  struct acpi_header header;
  uint32_t tables[];
} __attribute__((packed));
struct xsdt {
  struct acpi_header header;
  uint64_t tables[];
} __attribute__((packed));
#define GAS_ADDRESS_MEMORY 0
#define GAS_ADDRESS_IO 1
#define GAS_ADDRESS_PCI 2
struct generic_address_structure {
  uint8_t address_space;
  uint8_t bit_width;
  uint8_t bit_offset;
  uint8_t access_size;
  uint64_t address;
} __attribute__((packed));

struct acpi_header* acpi_find_table(const char*, bool, size_t);
void parse_acpi(void);

extern int acpi_revision;
