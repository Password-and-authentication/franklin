#ifndef _APIC_
#define _APIC_ 1

#include "../ACPI/acpi.h"

#define EOI_REG 0xB0
#define SPURIOUS_VECTOR 0xF0
#define TPR_REG 0x80
#define TIMER_REG 0x320
#define DIVIDE_REG 0x3E0
#define INITCOUNT 0x380
#define LINT0 0x350
#define LINT1 0x360


volatile uint32_t *EOI;


uint8_t NMI_LINT;

void apic(void);
void walk_madt(MADT*);
void init_apic(volatile uint32_t*);
void init_timer(volatile uint32_t*);

void sleep(int);

#endif
