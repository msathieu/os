#pragma once
#include <sys/types.h>
#define _SC_PAGESIZE 0
#define _SC_PAGE_SIZE 0

int close(int);
pid_t fork(void);
pid_t getpid(void);
pid_t getppid(void);
uid_t getuid(void);
ssize_t read(int fd, void* buffer, size_t size);
unsigned sleep(unsigned duration);
long sysconf(int name);
ssize_t write(int fd, const void* buffer, size_t size);
