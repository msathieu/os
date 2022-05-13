#pragma once
#include_next <elf.h>
#include <stddef.h>

void load_kernel(uintptr_t, size_t);

extern size_t kernel_size;
