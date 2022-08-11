#include <stdint.h>
#include "idt.h"
#include "limine.h"
#include "defs.h"
#include "../ACPI/acpi.h"
#include "../69.h"
#include "io.h"


extern void *isr_table[];
void set_idt_entry(uint8_t, void*, uint8_t);

__attribute__((aligned(0x10)))
static idt_entry idt[256];

void init_idt() {
    idtr.base = (uintptr_t)&idt[0];
    idtr.size = (uint16_t)sizeof(idt_entry) * IDT_MAX_DESC - 1;

    for (uint8_t vector = 0; vector < 34; vector++) {
        set_idt_entry(vector, isr_table[vector], 0x8E);
    }

    asm("lidt %0" :: "m" (idtr));
    asm("sti");
}

void load_idt() {
    asm("lidt %0" :: "m" (idtr));
    asm("sti");
}

char kbd_US[];

void kbd() {
    char keycode = in(0x60);
    print((char*)&kbd_US[keycode]);
    out(0x20, 0x20);
    return;
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
    // print("timer");
    // *EOI = 0;
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



char kbd_US [128] =
{
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',   
  '\t', /* <-- Tab */
  'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',     
    0, /* <-- control key */
  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',  0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0,
  '*',
    0,  /* Alt */
  ' ',  /* Space bar */
    0,  /* Caps lock */
    0,  /* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,  /* < ... F10 */
    0,  /* 69 - Num lock*/
    0,  /* Scroll Lock */
    0,  /* Home key */
    0,  /* Up Arrow */
    0,  /* Page Up */
  '-',
    0,  /* Left Arrow */
    0,
    0,  /* Right Arrow */
  '+',
    0,  /* 79 - End key*/
    0,  /* Down Arrow */
    0,  /* Page Down */
    0,  /* Insert Key */
    0,  /* Delete Key */
    0,   0,   0,
    0,  /* F11 Key */
    0,  /* F12 Key */
    0,  /* All other keys are undefined */
};