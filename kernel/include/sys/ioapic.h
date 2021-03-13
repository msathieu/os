#pragma once
#include <cpu/isr.h>

void register_isa_irq(int, isr_handler);
void unregister_isa_irq(int);
void setup_ioapics(void);
