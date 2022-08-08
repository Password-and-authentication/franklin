#include <stdint.h>
#include "idt.h"
#include "limine.h"
#include "defs.h"
#include "../ACPI/acpi.h"
#include "../69.h"


extern void *isr_table[];
void set_idt_entry(uint8_t, void*, uint8_t);

__attribute__((aligned(0x10)))
static idt_entry idt[256];

void init_idt() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.size = (uint16_t)sizeof(idt_entry) * IDT_MAX_DESC - 1;

    for (uint8_t vector = 0; vector < 33; vector++) {
        set_idt_entry(vector, isr_table[vector], 0x8E);
    }

    asm("lidt %0" :: "m" (idtr));
    asm("sti");
}

void load_idt() {
    asm("lidt %0" :: "m" (idtr));
    asm("sti");
}


void set_idt_entry(uint8_t vector, void* isr, uint8_t flags) {

    idt_entry *desc = &idt[vector];
    desc->isr_low = (uint64_t)isr & 0xFFFF;
    desc->selector = (0x5 << 3);
    desc->ist = 0;
    desc->attributes = flags;
    desc->isr_mid = ((uint64_t)isr >> 16) & 0xFFFF;
    desc->isr_high = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    desc->zero = 0;
}


void timerh(uint64_t t) {
    print("timer");
    *EOI = 0;
    return;
}

void exception_handler(uint64_t code) {
    print("\n\nError: ");
    char s[20];
    itoa(code, s);
    print(s);
    if (code == 0)
        print("Division By Zero\n");

    asm("cli; hlt");
};