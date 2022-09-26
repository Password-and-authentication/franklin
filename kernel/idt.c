#include "franklin/idt.h"
#include "asm/x86.h"
#include "d.h"
#include "franklin/acpi.h"
#include "franklin/defs.h"
#include "franklin/interrupt.h"
#include "franklin/spinlock.h"
#include "limine.h"
#include <stdint.h>

__attribute__((aligned(0x10))) static idt_entry idt[256];

static struct idtr
{
  uint16_t size;
  uint64_t base;
} __attribute__((packed)) idtr;

void
load_idt()
{

  asm("lidt %0" ::"m"(idtr));
}

void
init_idt()
{
  idtr.base = (uintptr_t)&idt[0];
  idtr.size = (uint16_t)sizeof(idt_entry) * IDT_MAX_DESC - 1;

  for (uint8_t vector = 0; vector < 32; vector++) {
    set_idt_entry(vector, isr_table[vector], 0x8E);
  }

  load_idt();
}

void
new_irq(uint8_t vector, void (*isr)(void))
{
  set_idt_entry(vector, isr, 0x8E);
}

void
set_idt_entry(uint8_t vector, void (*isr)(), uint8_t flags)
{

  idt_entry* desc = &idt[vector];
  desc->isr_low = (uint16_t)(uint64_t)isr;
  desc->selector = (0x5 << 3); // kernel code segment in GDT
  desc->ist = 0;
  desc->attributes = flags;
  desc->isr_mid = (uint16_t)((uint64_t)isr >> 16);
  desc->isr_high = (uint32_t)((uint64_t)isr >> 32);
  desc->zero = 0;
}
#include "franklin/switch.h"

struct regs;
extern uint32_t* EOI;
void
trap(struct regs* regs)
{
  if (regs->code < 32) {
    uint8_t s[20];
    itoa(regs->code, s);
    print("trap error: ");
    print(s);
    asm("cli; hlt");
  }

  switch (regs->code) {
    case 32:
      timerh(10);
      break;
    case 33:
      kbd_press();
      break;
    case 34:
      apic_timer(regs);
      break;
  }
};
