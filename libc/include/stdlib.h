#pragma once
#include <__/freestanding/stdlib.h>
#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0

void* aligned_alloc(size_t alignment, size_t size);
_Noreturn void exit(int status);
void free(void* ptr);
void* malloc(size_t size);
void* realloc(void* ptr, size_t size);
int setenv(const char* name, const char* value, int overwrite);
