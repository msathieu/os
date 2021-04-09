#pragma once
#include <sys/types.h>
#define O_RDONLY 1
#define O_RDWR 2
#define O_WRONLY 4
#define O_CREAT 8
#define O_TRUNC 16

int creat(const char* path, mode_t mode);
int open(const char* path, int flags, ...);
