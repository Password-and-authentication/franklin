#include <stdint.h>
#include "acpi.h"
#include "../kernel/limine.h"



static volatile struct limine_rsdp_request rsdp_req = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0,
};

#define L 0x43495041

void fuck() {
    int x = 10;
    int y = 20;
    return;
}

int acpi() {
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
    volatile uint32_t *vector = (uint32_t*)(HHDM_OFFSET + madt->lapic + 0xF0);
    *vector = 0x1FF;
    volatile uint32_t *tpr = (uint32_t*)(HHDM_OFFSET + madt->lapic + 0x80);
    EOI = (uint32_t*)(HHDM_OFFSET + madt->lapic + 0xB0);
    *tpr = 0;

    void initimer(MADT *madt);

    initimer(madt);


    uint8_t lapic[100];
    int x = 1;
    for (int i = 0; i < hdr->length;) {
        if (madt->entry[i] == 0)
            lapic[x++] = madt->entry[i + 3];
        i += madt->entry[i + 1];
    }
    
    
    return 69;
}


void initimer(MADT *madt) {

    volatile uint32_t *initcount = (uint32_t*)(HHDM_OFFSET + madt->lapic + 0x380);
    volatile uint32_t *timer = (uint32_t*)(HHDM_OFFSET + madt->lapic + 0x320);
    volatile uint32_t *divide = (uint32_t*)(HHDM_OFFSET + madt->lapic + 0x3E0);
    *timer = 1 << 17 | 32;
    *divide = 0x3;
    *initcount = 10000;


    




}

uint8_t checkchecksum(RSDP* rsdp) {

    uint64_t sum = 0;

    return 0;
}