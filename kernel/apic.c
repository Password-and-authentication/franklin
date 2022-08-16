#include <stdint.h>
#include "../ACPI//acpi.h"
#include "franklin/apic.h"
#include "franklin/69.h"
#include "franklin/io.h"
#include "franklin/idt.h"
#include "franklin/defs.h"




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

void write32(unsigned long base, unsigned int reg, unsigned int val) {
  *(volatile unsigned int*)(base + reg) = val;
}

unsigned int* read32(unsigned long base, unsigned int reg) {
  return (unsigned int*)(base + reg);
}


void init_apic(uint32_t* lapic) {
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
extern void isr_apic_timer(void);


void init_timer(uint32_t* lapic) {

    init_pit(1000); // how many interrupts every second
    new_irq(34, isr_apic_timer);

    
    write32(lapic, DIVIDE_REG, 0); // divide by 2
    write32(lapic, INITCOUNT, ~0);

    sleep(1); // sleep for 1 ms

    write32(lapic, TIMER_REG, 1 << 16); // stop timer
    unsigned int ticks = ~0 - *read32(lapic, CURRENTCOUNT);
    ticks *= 100; // interrupt every 10ms

    print("start lapic timer\n");
    write32(lapic, TIMER_REG, 34 | 1 << 17); // vector 34 and periodic mode
    write32(lapic, DIVIDE_REG, 0);
    write32(lapic, INITCOUNT, ticks);
}


void apic_timer() {
    static int i;




    *EOI = 0;
}
extern void isr_timer(void);

void init_pit(int hz) {

  new_irq(32, isr_timer);
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





