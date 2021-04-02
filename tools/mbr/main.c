#include <stdint.h>
#include <stdio.h>
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

int main(void) {
  FILE* iso_file = fopen("os.iso", "r+");
  if (!iso_file) {
    puts("Can't open os.iso");
    return 1;
  }
  if (!fread(&mbr, sizeof(struct mbr), 1, iso_file)) {
    puts("Error reading file");
    return 1;
  }
  mbr.partitions[1].type = MBR_TYPE_LVM;
  mbr.partitions[1].lba_start = mbr.partitions[0].lba_start + mbr.partitions[0].num_sectors;
  fseek(iso_file, 0, SEEK_END);
  size_t iso_size = ftell(iso_file);
  mbr.partitions[1].num_sectors = (iso_size + 511) / 512 - mbr.partitions[1].lba_start;
  rewind(iso_file);
  fwrite(&mbr, sizeof(struct mbr), 1, iso_file);
  fclose(iso_file);
}
