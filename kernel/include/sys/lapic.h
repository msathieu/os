#pragma once
#include <stdbool.h>
#include <stddef.h>

extern size_t madt_bsp_lapic_id;

void lapic_eoi(void);
void set_lapic_timer(size_t);
void setup_lapic(size_t);
void setup_lapic_timer(bool);
void start_aps(void);
