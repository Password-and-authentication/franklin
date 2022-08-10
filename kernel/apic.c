#include <stdint.h>
#include "../ACPI//acpi.h"
#include "apic.h"



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


void init_apic(uint32_t* lapic) {
    // set the correct LINT pin for NMI
    if (NMI_LINT == 1) {
        *(uint32_t*)((uint64_t)lapic + LINT0) = 1 << 17; // mask LINT0
        *(uint32_t*)((uint64_t)lapic + LINT1) = 1 << 10; // delivery mode: NMI
    } else {
        *(uint32_t*)((uint64_t)lapic + LINT0) = 1 << 10;
        *(uint32_t*)((uint64_t)lapic + LINT1) = 1 << 17;
    }
    EOI = (uint32_t*)((uint64_t)lapic + EOI_REG);
    *(uint32_t*)((uint64_t)lapic + TPR_REG) = 0;
    *(uint32_t*)((uint64_t)lapic + SPURIOUS_VECTOR) = 0x1FF;

    init_timer(lapic);
}


void init_timer(uint32_t* lapic) {
    *(uint32_t*)((uint64_t)lapic + TIMER_REG) = 1 << 17 | 32; // periodic mode and vector 32
    *(uint32_t*)((uint64_t)lapic + DIVIDE_REG) = 0x3;
    *(uint32_t*)((uint64_t)lapic + INITCOUNT) = 10000;
}
