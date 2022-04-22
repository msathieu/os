#pragma once
#include <sys/acpi.h>
#define KERNEL_VIRTUAL_ADDRESS 0xffffffff80000000

struct loader_memory_map {
  uint64_t address;
  uint64_t size;
};
struct loader_file {
  uint64_t address;
  uint64_t size;
  uint8_t name[128];
};
struct loader_struct {
  uint32_t version;
  uint64_t kernel_physical_address;
  struct loader_memory_map memory_map[1024];
  struct acpi_rsdp rsdp;
  struct loader_file files[128];
  uint64_t fb_address;
  uint32_t fb_width;
  uint32_t fb_height;
  uint32_t fb_pitch;
  uint32_t fb_bpp;
  uint16_t fb_red_index;
  uint16_t fb_green_index;
  uint16_t fb_blue_index;
  uint16_t debug_port;
} __attribute__((packed));

extern struct loader_struct loader_struct;
