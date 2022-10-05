#include "franklin/interrupt.h"
#include "asm/x86.h"
#include "franklin/69.h"
#include "franklin/idt.h"
#include "franklin/kbd.h"
#include "franklin/pic.h"
#include "franklin/switch.h"
#include <stdint.h>

extern void
spurious(void);

extern void
isr_syscall(void);

void
init_interrupt()
{
  init_idt();
  pic_remap(0x20);
  new_irq(33, isr_kbd);
  new_irq(34, isr_apic_timer);
  new_irq(32, isr_timer);
  new_irq(255, spurious);
  new_irq(69, isr_syscall);
  set_idt_entry(69, isr_syscall, 0xEE);
  unmask_irq(1);
  unmask_irq(0);
  init_kbd(); // init ps/2 keyboard
}

void
timerh()
{
  PIT_COUNTER--;

  out(0x20, 0x20);
  return;
}
