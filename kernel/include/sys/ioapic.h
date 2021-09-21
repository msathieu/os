#pragma once
#include <cpu/isr.h>
#include <stdbool.h>

void map_gsi(int, int, bool, bool);
void register_isa_irq(int, isr_handler);
void unregister_isa_irq(int);
void setup_ioapics(void);
