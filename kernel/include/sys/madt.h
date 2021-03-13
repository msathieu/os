#pragma once
#include <stddef.h>
#include <sys/acpi.h>

void parse_madt(struct acpi_header*);

extern uintptr_t madt_lapic_address;
extern struct acpi_madt_lapic* madt_lapics[256];
extern size_t madt_num_lapics;
extern struct acpi_madt_ioapic* madt_ioapics[256];
extern size_t madt_num_ioapics;
extern struct acpi_madt_interrupt_override* madt_overrides[16];
extern size_t madt_num_overrides;
extern struct acpi_madt_nmi* madt_nmis[256];
extern size_t madt_num_nmis;
