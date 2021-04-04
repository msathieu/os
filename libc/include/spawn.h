#pragma once
#include <stdint.h>
#include <sys/types.h>

void add_argument(const char* arg);
void grant_capability(int namespace, int capability);
void grant_ioport(uint16_t port);
void register_irq(int irq);
pid_t spawn_process(const char* file);
pid_t spawn_process_raw(const char* file);
void start_process(void);
