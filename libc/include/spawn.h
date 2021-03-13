#pragma once
#include <stdint.h>
#include <sys/types.h>

void add_argument(const char*);
void grant_capability(int, int);
void grant_ioport(uint16_t);
uintptr_t map_physical_memory(uintptr_t, size_t);
void register_irq(int);
pid_t spawn_process(const char*);
pid_t spawn_process_raw(const char*);
void start_process(void);
