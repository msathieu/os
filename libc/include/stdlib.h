#pragma once
#include <__/freestanding/stdlib.h>
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

void* aligned_alloc(size_t, size_t);
_Noreturn void exit(int);
void free(void*);
char* getenv(const char*);
void* malloc(size_t);
void* realloc(void*, size_t);
int setenv(const char*, const char*, int);
