#include <stdint.h>
#include "acpi.h"
#include "../kernel/limine.h"



static volatile struct limine_rsdp_request rsdp_req = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0,
};

#define L 0x43495041


int acpi(void) {
    struct limine_rsdp_response *rsdp_res = rsdp_req.response;
    RSDP *rsdp = (RSDP*) rsdp_res->address;
    RSDT *rsdt =  (uintptr_t) rsdp->rsdtaddr + HHDM_OFFSET;
    defaultheader *hdr = rsdt->entry[0] + HHDM_OFFSET;
    int i = 0;

    while ((hdr = rsdt->entry[i++] + HHDM_OFFSET)) {
        if (hdr->signature == L)
            break;
    }
    MADT *madt = rsdt->entry[--i] + HHDM_OFFSET;

    uint8_t lapicId[100], NMI, x = 1;
    for (int i = 0; i < hdr->length;) {
        if (madt->entry[i] == 0)
            lapicId[x++] = madt->entry[i + 3];
        if (madt->entry[i] == 4)
            NMI = madt->entry[i + 5];
        i += madt->entry[i + 1];
    }
    init_apic(HHDM_OFFSET + madt->lapic, NMI);
    
    return 69;
}



void init_apic(uint32_t* lapic, uint8_t NMI) {

    // set the correct LINT pin for NMI
    if (NMI == 1) {
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
    *(uint32_t*)((uint64_t)lapic + TIMER_REG) = 1 << 17 | 32;
    *(uint32_t*)((uint64_t)lapic + DIVIDE_REG) = 0x3;
    *(uint32_t*)((uint64_t)lapic + INITCOUNT) = 10000;
}
