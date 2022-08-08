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
    init_apic(HHDM_OFFSET + madt->lapic);

    uint8_t lapic[100];
    int x = 1;
    for (int i = 0; i < hdr->length;) {
        if (madt->entry[i] == 0)
            lapic[x++] = madt->entry[i + 3];
        i += madt->entry[i + 1];
    }
    
    return 69;
}



void init_apic(uint32_t* lapic) {
    EOI = (uint32_t*)((uint64_t)lapic + EOI_REG);
    *(uint32_t*)((uint64_t)lapic + TPR_REG) = 0;
    *(uint32_t*)((uint64_t)lapic + SPURIOUS_VECTOR) = 0x1FF;
    *(uint32_t*)((uint64_t)lapic + LINT0) = 1 << 17;

    init_timer(lapic);
}


void init_timer(uint32_t* lapic) {
    *(uint32_t*)((uint64_t)lapic + TIMER_REG) = 1 << 17 | 32;
    *(uint32_t*)((uint64_t)lapic + DIVIDE_REG) = 0x3;
    *(uint32_t*)((uint64_t)lapic + INITCOUNT) = 10000;
}
