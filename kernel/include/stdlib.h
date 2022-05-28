#pragma once
#include_next <stdlib.h>

void free(void*);
void* malloc(size_t);
void* realloc(void* ptr, size_t size);
void* valloc(size_t);
