#include <elf.h>
#include <panic.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <struct.h>

struct mb_tag {
  uint32_t type;
  uint32_t size;
};
#define MULTIBOOT_MODULE_TAG 3
struct mb_module_tag {
  uint32_t type;
  uint32_t tag_size;
  uint32_t start;
  uint32_t end;
  uint8_t name[];
};
#define MULTIBOOT_MEMORY_MAP_TAG 6
#define MULTIBOOT_MEMORY_AVAILABLE 1
struct mb_memory_map_entry {
  uint64_t address;
  uint64_t size;
  uint32_t type;
  uint32_t reserved;
};
struct mb_memory_map_tag {
  uint32_t type;
  uint32_t size;
  uint32_t entry_size;
  uint32_t version;
};
#define MULTIBOOT_FRAMEBUFFER_TAG 8
struct mb_framebuffer_tag {
  uint32_t type;
  uint32_t size;
  uint64_t address;
  uint32_t pitch;
  uint32_t width;
  uint32_t height;
  uint8_t bits_per_pixel;
  uint8_t color_type;
  uint16_t reserved;
  uint8_t red_pos;
  uint8_t red_size;
  uint8_t green_pos;
  uint8_t green_size;
  uint8_t blue_pos;
  uint8_t blue_size;
};
#define MULTIBOOT_OLD_RSDP_TAG 14
struct acpi_rsdp {
  uint8_t signature[8];
  uint8_t checksum;
  uint8_t oem_id[6];
  uint8_t revision;
  uint32_t rsdt;
};
struct mb_old_rsdp_tag {
  uint32_t type;
  uint32_t size;
  struct acpi_rsdp rsdp;
};
#define MULTIBOOT_RSDP_TAG 15
struct mb_rsdp_tag {
  uint32_t type;
  uint32_t size;
  struct acpi_rsdp_extended rsdp;
};

static uintptr_t mbptr;
uintptr_t modules_end_addr;

static void* find_mb_tag(int type, bool required) {
  for (struct mb_tag* tag = (struct mb_tag*) (mbptr + 8); tag->type; tag = (struct mb_tag*) ((uintptr_t) tag + (tag->size + 7) / 8 * 8)) {
    if ((int) tag->type == type) {
      return tag;
    }
  }
  if (required) {
    printf("Multiboot tag %d not found\n", type);
    panic("Multiboot tag not found");
  }
  return 0;
}
void parse_multiboot_header(uintptr_t ptr) {
  mbptr = ptr;
  struct mb_memory_map_tag* memory_map = find_mb_tag(MULTIBOOT_MEMORY_MAP_TAG, 1);
  size_t j = 0;
  for (
    struct mb_memory_map_entry* entry = (struct mb_memory_map_entry*) ((uintptr_t) memory_map + sizeof(struct mb_memory_map_tag));
    (uintptr_t) entry < (uintptr_t) memory_map + memory_map->size;
    entry = (struct mb_memory_map_entry*) ((uintptr_t) entry + memory_map->entry_size)) {
    if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
      uint64_t start = entry->address;
      uint64_t end = start + entry->size;
      start = (start + 0xfff) / 0x1000 * 0x1000;
      end = end / 0x1000 * 0x1000;
      if (end > start) {
        loader_struct.memory_map[j].address = start;
        loader_struct.memory_map[j].size = end - start;
        j++;
      }
    }
  }
  uintptr_t kernel_start = 0;
  size_t kernel_mod_size;
  size_t file_i = 0;
  for (struct mb_tag* tag = (struct mb_tag*) (mbptr + 8); tag->type; tag = (struct mb_tag*) ((uintptr_t) tag + (tag->size + 7) / 8 * 8)) {
    if (tag->type == MULTIBOOT_MODULE_TAG) {
      struct mb_module_tag* modtag = (struct mb_module_tag*) tag;
      if (modtag->end > modules_end_addr) {
        modules_end_addr = modtag->end;
      }
      if (!strcmp((char*) modtag->name, "kernel")) {
        kernel_start = modtag->start;
        kernel_mod_size = modtag->end - modtag->start;
      } else {
        loader_struct.files[file_i].address = modtag->start;
        loader_struct.files[file_i].size = modtag->end - modtag->start;
        strcpy((char*) loader_struct.files[file_i++].name, (char*) modtag->name);
      }
    }
  }
  if (!modules_end_addr) {
    panic("No modules loaded");
  }
  if (modules_end_addr < 0x100000) {
    panic("Last module is located before loader, this shouldn't happen");
  }
  if (!kernel_start) {
    panic("No kernel loaded");
  }
  load_kernel(kernel_start, kernel_mod_size);
  struct mb_rsdp_tag* rsdp_tag = find_mb_tag(MULTIBOOT_RSDP_TAG, 0);
  if (rsdp_tag) {
    memcpy(&loader_struct.rsdp, &rsdp_tag->rsdp, sizeof(struct acpi_rsdp_extended));
  } else {
    struct mb_old_rsdp_tag* old_rsdp_tag = find_mb_tag(MULTIBOOT_OLD_RSDP_TAG, 1);
    memcpy(&loader_struct.rsdp, &old_rsdp_tag->rsdp, sizeof(struct acpi_rsdp));
  }
  if (memcmp(loader_struct.rsdp.signature, "RSD PTR ", 8)) {
    panic("Invalid RSDP signature");
  }
  uint8_t checksum = 0;
  for (size_t i = 0; i < sizeof(struct acpi_rsdp); i++) {
    checksum += ((uint8_t*) &loader_struct.rsdp)[i];
  }
  if (checksum) {
    panic("Invalid RSDP checksum");
  }
  if (rsdp_tag) {
    for (size_t i = sizeof(struct acpi_rsdp); i < rsdp_tag->rsdp.size; i++) {
      checksum += ((uint8_t*) &loader_struct.rsdp)[i];
    }
    if (checksum) {
      panic("Invalid RSDP extended checksum");
    }
  }
  struct mb_framebuffer_tag* fb_tag = find_mb_tag(MULTIBOOT_FRAMEBUFFER_TAG, 1);
  if (fb_tag->color_type != 1) {
    panic("Framebuffer uses unsupported color mode");
  }
  if (fb_tag->red_size != 8 || fb_tag->green_size != 8 || fb_tag->blue_size != 8) {
    panic("Framebuffer uses unsupported color size");
  }
  if (fb_tag->red_pos % 8 || fb_tag->green_pos % 8 || fb_tag->blue_pos % 8) {
    panic("Framebuffer uses unsupported color addressing");
  }
  loader_struct.fb_address = fb_tag->address;
  loader_struct.fb_width = fb_tag->width;
  loader_struct.fb_height = fb_tag->height;
  loader_struct.fb_pitch = fb_tag->pitch;
  loader_struct.fb_bpp = fb_tag->bits_per_pixel;
  loader_struct.fb_red_index = fb_tag->red_pos / 8;
  loader_struct.fb_green_index = fb_tag->green_pos / 8;
  loader_struct.fb_blue_index = fb_tag->blue_pos / 8;
}
