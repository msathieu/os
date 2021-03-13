#include <dirent.h>
#include <monocypher.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>
#include <sys/stat.h>

static void generate(void) {
  uint8_t private[32];
  if (getrandom(private, 32, 0) != 32) {
    puts("Can't obtain enough randomness");
    exit(1);
  }
  FILE* private_file = fopen("private.key", "w");
  fwrite(private, 1, 32, private_file);
  fclose(private_file);
  uint8_t public[32];
  crypto_sign_public_key(public, private);
  crypto_wipe(private, 32);
  FILE* public_file = fopen("public.key", "w");
  fwrite(public, 1, 32, public_file);
  fclose(public_file);
}
static void sign(void) {
  FILE* private_file = fopen("private.key", "r");
  if (!private_file) {
    puts("Can't open private key");
    return;
  }
  uint8_t private[32];
  if (fread(private, 1, 32, private_file) != 32) {
    puts("Invalid private key");
    exit(1);
  }
  fclose(private_file);
  FILE* public_file = fopen("public.key", "r");
  if (!public_file) {
    puts("Can't open public key");
    return;
  }
  uint8_t public[32];
  if (fread(public, 1, 32, public_file) != 32) {
    puts("Invalid public key");
    exit(1);
  }
  fclose(public_file);
  DIR* directory = opendir("sysroot/boot");
  struct dirent* entry;
  while ((entry = readdir(directory))) {
    if (strstr(entry->d_name, ".sig") || !strcmp(entry->d_name, "loader.bin")) {
      continue;
    }
    char abs_name[strlen("sysroot/boot/") + strlen(entry->d_name) + 1];
    strcpy(abs_name, "sysroot/boot/");
    strcat(abs_name, entry->d_name);
    struct stat entry_stat;
    stat(abs_name, &entry_stat);
    if (S_ISDIR(entry_stat.st_mode)) {
      continue;
    }
    FILE* executable_file = fopen(abs_name, "r");
    if (!executable_file) {
      puts("Can't open executable");
      exit(1);
    }
    fseek(executable_file, 0, SEEK_END);
    size_t size = ftell(executable_file);
    rewind(executable_file);
    uint8_t* executable = malloc(size);
    if (fread(executable, 1, size, executable_file) != size) {
      puts("Error reading executable");
      exit(1);
    }
    fclose(executable_file);
    uint8_t signature[64];
    crypto_sign(signature, private, public, executable, size);
    free(executable);
    char signature_path[strlen(abs_name) + 5];
    strcpy(signature_path, abs_name);
    strcat(signature_path, ".sig");
    FILE* signature_file = fopen(signature_path, "w");
    fwrite(signature, 1, 64, signature_file);
    fclose(signature_file);
  }
  crypto_wipe(private, 32);
}
int main(int argc, char* argv[]) {
  if (argc != 2) {
    puts("One argument is needed");
    return 1;
  }
  if (!strcmp(argv[1], "generate")) {
    generate();
  } else if (!strcmp(argv[1], "sign")) {
    sign();
  } else {
    puts("Invalid argument");
    return 1;
  }
}
