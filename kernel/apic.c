#include <stdint.h>
#include "../ACPI//acpi.h"
#include "apic.h"
#include "../69.h"
#include "io.h"
#include "idt.h"
#include "defs.h"


// right now its only getting the LINT pin that is connected to NMI
void walk_madt(MADT *madt) {

    char lapicId[100], x = 1;
    for (int i = 0; i < madt->h.length;) {
        if (madt->entry[i] == 0)
            lapicId[x++] = madt->entry[i + 3];
        if (madt->entry[i] == 4)
            NMI_LINT = madt->entry[i + 5];
        i += madt->entry[i + 1];
    }
}

void write32(unsigned int base, unsigned int reg, unsigned int val) {
  *(volatile unsigned int*)((unsigned long)base + reg) = val;
}

unsigned int* read32(unsigned int base, unsigned int reg) {
  return (unsigned int*)((unsigned long)base + reg);
}


void init_apic(volatile uint32_t* lapic) {
    // set the correct LINT pin for NMI
    if (NMI_LINT == 1) {
      write32(lapic, LINT1, 1 << 10);
    } else {
      write32(lapic, LINT0, 1 << 10);
    };
    EOI = read32(lapic, EOI_REG);
    write32(lapic, TPR_REG, 0);
    write32(lapic, SPURIOUS_VECTOR, 0x1FF);

    init_timer(lapic);
}

void init_pit(int);


void init_timer(volatile uint32_t* lapic) {

    init_pit(1000); // how many interrupts every second
    
}

void apic_timer() {
    static int i;





    *EOI = 0;
}


void init_pit(int hz) {

    int divisor = 1193180 / hz;
    out(0x43, 0b110100);
    out(0x40, divisor & 0xFF);
    out(0x40, divisor >> 8);
}


void sleep(int us) {
    countdown = us;

    while (countdown > 0)
        ;
}





