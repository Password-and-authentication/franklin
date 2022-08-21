#ifndef _INTERRUPT_
#define _INTERRUPT_

#include "switch.h"

extern void isr_timer(void);
extern void isr_apic_timer(void);
extern void isr_kbd(void);

extern void apic_timer(regs_t*);
extern void kbd_press(void);
extern void timerh(unsigned long);


void init_interrupt(void);



volatile int PIT_COUNTER;


#endif
