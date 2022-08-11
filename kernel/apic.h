#ifndef _APIC_
#define _APIC_ 1

#include "../ACPI/acpi.h"


uint8_t NMI_LINT;

void apic(void);
void walk_madt(MADT*);
void init_apic(volatile uint32_t*);
void init_timer(volatile uint32_t*);

#endif
