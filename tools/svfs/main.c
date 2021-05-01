#include <dirent.h>
#include <monocypher.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#define SVFS_MAGIC 0x55ce581be6Bfa5a6

enum {
  TYPE_FILE,
  TYPE_DIR
};
struct svfs_file {
  uint8_t name[256];
  uint64_t offset;
  uint64_t size;
  uint8_t hash[64];
  uint8_t type;
};
struct svfs_header {
  uint8_t signature[64];
  uint64_t magic;
  uint32_t version;
  uint32_t nfiles;
  struct svfs_file files[];
};

static struct svfs_header* header;
static FILE* img;

static void loop_directory(const char* path, int step) {
  DIR* directory = opendir(path);
  struct dirent* entry;
  while ((entry = readdir(directory))) {
    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) {
      continue;
    }
    char abs_name[strlen(path) + 1 + strlen(entry->d_name) + 1];
    strcpy(abs_name, path);
    strcat(abs_name, "/");
    strcat(abs_name, entry->d_name);
    struct stat entry_stat;
    stat(abs_name, &entry_stat);
    if (S_ISDIR(entry_stat.st_mode)) {
      loop_directory(abs_name, step);
    }
    if (!step) {
      header->nfiles++;
      header = realloc(header, sizeof(struct svfs_header) + header->nfiles * sizeof(struct svfs_file));
      memset(&header->files[header->nfiles - 1], 0, sizeof(struct svfs_file));
      strncpy((char*) header->files[header->nfiles - 1].name, abs_name + strlen("system/"), 256);
      header->files[header->nfiles - 1].type = TYPE_DIR;
      header->files[header->nfiles - 1].offset = header->files[header->nfiles - 2].offset + header->files[header->nfiles - 2].size;
      if (!S_ISDIR(entry_stat.st_mode)) {
        header->files[header->nfiles - 1].type = TYPE_FILE;
        header->files[header->nfiles - 1].size = entry_stat.st_size;
        uint8_t* content = malloc(entry_stat.st_size);
        FILE* file = fopen(abs_name, "r");
        if (fread(content, 1, entry_stat.st_size, file) != (size_t) entry_stat.st_size) {
          puts("Error reading file");
          exit(1);
        }
        fclose(file);
        crypto_blake2b(header->files[header->nfiles - 1].hash, content, entry_stat.st_size);
        free(content);
      }
    } else {
      if (!S_ISDIR(entry_stat.st_mode)) {
        uint8_t* content = malloc(entry_stat.st_size);
        FILE* file = fopen(abs_name, "r");
        if (fread(content, 1, entry_stat.st_size, file) != (size_t) entry_stat.st_size) {
          puts("Error reading file");
          exit(1);
        }
        fclose(file);
        fwrite(content, 1, entry_stat.st_size, img);
        free(content);
      }
    }
  }
  closedir(directory);
}
int main(void) {
  header = calloc(1, sizeof(struct svfs_header) + sizeof(struct svfs_file));
  header->magic = SVFS_MAGIC;
  header->version = 0;
  header->nfiles = 1;
  header->files[0].type = TYPE_DIR;
  loop_directory("system", 0);
  FILE* private_file = fopen("private.key", "r");
  FILE* public_file = fopen("public.key", "r");
  if (private_file && public_file) {
    uint8_t private[32];
    if (fread(private, 1, 32, private_file) != 32) {
      puts("Invalid private key");
      return 1;
    }
    fclose(private_file);
    uint8_t public[32];
    if (fread(public, 1, 32, public_file) != 32) {
      puts("Invalid public key");
      return 1;
    }
    fclose(public_file);
    crypto_sign(header->signature, private, public, (uint8_t*) header + offsetof(struct svfs_header, magic), sizeof(struct svfs_header) - offsetof(struct svfs_header, magic) + header->nfiles * sizeof(struct svfs_file));
    crypto_wipe(private, 32);
  }
  img = fopen("svfs.img", "w");
  fwrite(header, sizeof(struct svfs_header) + header->nfiles * sizeof(struct svfs_file), 1, img);
  loop_directory("system", 1);
  fclose(img);
}
