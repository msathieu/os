#pragma once
#include <stdbool.h>
#include <stddef.h>

void* heap_alloc(size_t, bool);
void setup_heap(void);

extern bool heap_enabled;
