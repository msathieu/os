#pragma once
#include <stdbool.h>
#include <stddef.h>

extern size_t madt_bsp_lapic_id;
extern bool broadcasted_nmi;

size_t get_current_lapic_id(void);
void lapic_eoi(void);
void set_lapic_timer(size_t);
void setup_lapic(size_t);
void setup_lapic_timer(bool);
void smp_broadcast_nmi(void);
void start_aps(void);
