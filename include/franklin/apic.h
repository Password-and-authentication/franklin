#ifndef _APIC_
#define _APIC_

#include "acpi.h"
#include "switch.h"

#define EOI_REG 0xB0
#define SPURIOUS_VECTOR 0xF0
#define TPR_REG 0x80
#define TIMER_REG 0x320
#define DIVIDE_REG 0x3E0
#define INITCOUNT 0x380
#define CURRENTCOUNT 0x390
#define LINT0 0x350
#define LINT1 0x360


volatile unsigned int *EOI;
char NMI_LINT;


void walk_madt(MADT*);
void init_apic(unsigned int*);
void apic_timer(regs_t*);



#endif
