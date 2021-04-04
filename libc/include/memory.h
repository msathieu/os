#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#define PHYSICAL_MAPPINGS_START 0x7fffc0000000

uintptr_t map_physical_memory(uintptr_t address, size_t size, bool child);
