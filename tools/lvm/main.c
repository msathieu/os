#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#define LVM_MAGIC 0x10f2af8ac29eb55
#define SVFS_MAGIC 0x55ce581be6Bfa5a6
#define TMP_MAGIC 0x5381835340924865
#define LVM_SECTOR (1024 * 1024)

struct lvm_volume {
  uint64_t filesystem;
  uint64_t size;
  uint32_t sectors[1024];
};
struct lvm_header {
  uint64_t magic;
  uint32_t version;
  uint32_t volumes[1024];
};

static struct lvm_header header = {LVM_MAGIC, 0, {}};
static size_t next_sector = 1;
static FILE* lvm_file;

static void add_svfs(void) {
  struct lvm_volume svfs_volume = {0};
  svfs_volume.filesystem = SVFS_MAGIC;
  FILE* svfs_file = fopen("svfs.img", "r");
  if (!svfs_file) {
    puts("Can't open svfs.img");
    exit(1);
  }
  fseek(svfs_file, 0, SEEK_END);
  svfs_volume.size = ftell(svfs_file);
  rewind(svfs_file);
  for (size_t i = 0; i < svfs_volume.size; i += LVM_SECTOR) {
    svfs_volume.sectors[i / LVM_SECTOR] = next_sector;
    uint8_t data[LVM_SECTOR] = {0};
    if (!fread(data, 1, LVM_SECTOR, svfs_file)) {
      puts("Error reading file");
      exit(1);
    }
    fseek(lvm_file, next_sector * LVM_SECTOR, SEEK_SET);
    fwrite(data, 1, LVM_SECTOR, lvm_file);
    next_sector++;
  }
  fclose(svfs_file);
  fseek(lvm_file, next_sector * LVM_SECTOR, SEEK_SET);
  fwrite(&svfs_volume, sizeof(struct lvm_volume), 1, lvm_file);
  header.volumes[0] = next_sector;
  next_sector++;
}
static void add_tmp(void) {
  struct lvm_volume tmp_volume = {0};
  tmp_volume.filesystem = TMP_MAGIC;
  fseek(lvm_file, next_sector * LVM_SECTOR, SEEK_SET);
  fwrite(&tmp_volume, sizeof(struct lvm_volume), 1, lvm_file);
  header.volumes[1] = next_sector;
  next_sector++;
}
int main(void) {
  lvm_file = fopen("lvm.img", "w");
  add_svfs();
  add_tmp();
  fseek(lvm_file, 0, SEEK_SET);
  fwrite(&header, sizeof(struct lvm_header), 1, lvm_file);
  fseek(lvm_file, 0, SEEK_END);
  size_t size = ftell(lvm_file);
  if (size % 512) {
    uint8_t null[512] = {0};
    fwrite(null, 1, 512 - size % 512, lvm_file);
  }
  fclose(lvm_file);
}
