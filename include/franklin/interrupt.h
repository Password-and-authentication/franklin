#ifndef _INTERRUPT_
#define _INTERRUPT_



extern void isr_timer(void);
extern void isr_apic_timer(void);
extern void isr_kbd(void);

void init_interrupt(void);

volatile int PIT_COUNTER;


#endif
