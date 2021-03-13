#pragma once
#include <stdbool.h>
#include <stddef.h>

extern volatile bool ap_startup;
extern size_t ap_nlapic;

void set_cpu_flags(void);
