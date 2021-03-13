#pragma once
#include <sys/types.h>
#define _SC_PAGESIZE 0
#define _SC_PAGE_SIZE 0

pid_t getpid(void);
pid_t getppid(void);
uid_t getuid(void);
unsigned sleep(unsigned);
long sysconf(int);
