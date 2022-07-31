#include <stdint.h>
#include "idt.h"



extern void *isr_table[];
void set_idt_entry(uint8_t, void*, uint8_t);

__attribute__((aligned(0x10)))
static idt_entry idt[256];
static idtr_t idtr;

void init_idt() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.size = (uint16_t)sizeof(idt_entry) * IDT_MAX_DESC - 1;

    for (uint8_t vector = 0; vector < 32; vector++) {
        set_idt_entry(vector, isr_table[vector], 0x8E);
    }

    __asm__ volatile("lidt %0" : : "m"(idtr));
    __asm__ volatile("sti");
}

void set_idt_entry(uint8_t vector, void* isr, uint8_t flags) {

    idt_entry *desc = &idt[vector];

    desc->isr_low = (uint64_t)isr & 0xFFFF;
    desc->selector = 0;
    desc->ist = 0;
    desc->isr_mid = ((uint64_t)isr >> 16) & 0xFFFF;
    desc->isr_high = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    desc->zero = 0;
}

void exception_handler(void) {
    __asm__ volatile("cli; hlt");
};