#pragma once
#include <stdint.h>

void parse_multiboot_header(uintptr_t);

extern uintptr_t modules_end_addr;
extern char* loader_cmd;
