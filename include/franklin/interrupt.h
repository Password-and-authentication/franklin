#ifndef _INTERRUPT_
#define _INTERRUPT_

#include "switch.h"

extern void
isr_timer(void);
extern void
isr_apic_timer(void);
extern void
isr_kbd(void);

extern void
apic_timer(struct regs*);
extern void
kbd_press(void);
extern void
timerh();

void
init_interrupt(void);

volatile uint32_t PIT_COUNTER;

#endif
