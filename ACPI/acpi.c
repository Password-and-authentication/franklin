#include <stdint.h>
#include "acpi.h"
#include "../kernel/limine.h"


static volatile struct limine_rsdp_request rsdp_req = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0,
};

#define L 0x43495041

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
    uint8_t lapic[100];
    int x = 0;
    for (int i = 0; i < hdr->length;) {
        if (madt->entry[i] == 0)
            lapic[x++] = madt->entry[i + 3];
        i += madt->entry[i + 1];
    }
    
    
    return 69;
}




uint8_t checkchecksum(RSDP* rsdp) {

    uint64_t sum = 0;

    return 0;
}