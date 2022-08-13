#include <stdint.h>
#include "../ACPI//acpi.h"
#include "apic.h"
#include "../69.h"
#include "io.h"
#include "idt.h"
#include "defs.h"


// right now its only getting the LINT pin that is connected to NMI
void walk_madt(MADT *madt) {

    uint8_t lapicId[100], x = 1;
    for (int i = 0; i < madt->h.length;) {
        if (madt->entry[i] == 0)
            lapicId[x++] = madt->entry[i + 3];
        if (madt->entry[i] == 4)
            NMI_LINT = madt->entry[i + 5];
        i += madt->entry[i + 1];
    }
}


void init_apic(volatile uint32_t* lapic) {
    // set the correct LINT pin for NMI
    if (NMI_LINT == 1) {
        *(volatile uint32_t*)((uint64_t)lapic + LINT1) = 1 << 10; // delivery mode: NMI
    } else {
        *(volatile uint32_t*)((uint64_t)lapic + LINT0) = 1 << 10;
    }
    EOI = (uint32_t*)((uint64_t)lapic + EOI_REG);
    *(volatile uint32_t*)((uint64_t)lapic + TPR_REG) = 0;
    *(volatile uint32_t*)((uint64_t)lapic + SPURIOUS_VECTOR) = 0x1FF;

    init_timer(lapic);
}

void init_pit(int);


void init_timer(volatile uint32_t* lapic) {
    // *(volatile uint32_t*)((uint64_t)lapic + TIMER_REG) = 1 << 17 | 32; // periodic mode and vector 32
    // *(volatile uint32_t*)((uint64_t)lapic + DIVIDE_REG) = 0x3;
    // *(volatile uint32_t*)((uint64_t)lapic + INITCOUNT) = 2500000; // not configured yet

    init_pit(1000);
}


void init_pit(int hz) {

    int divisor = 1193180 / hz;
    out(0x43, 0b110100);
    out(0x40, divisor & 0xFF);
    out(0x40, divisor >> 8);
}


void sleep(int ms) {
    countdown = ms;

    while (countdown > 0)
        ;
}





