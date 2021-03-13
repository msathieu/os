#pragma once
#include <cpu/isr.h>
#include <stddef.h>
#include <sys/acpi.h>
#define TIME_MILLISECOND 1000000000000

size_t get_time(void);
void setup_hpet(struct acpi_header*);
void sleep(size_t);
void sleep_current_task(size_t, struct isr_registers*);
