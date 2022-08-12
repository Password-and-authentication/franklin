#include <stdint.h>
#include "acpi.h"
#include "../kernel/limine.h"



static volatile struct limine_rsdp_request rsdp_req = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0,
};

#define L 0x43495041



void init_acpi() {
    struct limine_rsdp_response *rsdp_res = rsdp_req.response;
    RSDP *rsdp = (RSDP*) rsdp_res->address;
    rsdt = (uintptr_t)rsdp->rsdtaddr + HHDM_OFFSET;
}


void* get_acpi_sdt(uint64_t signature) {
    defaultheader *hdr = rsdt->entry[0] + HHDM_OFFSET;
    int i = 0;
    while ((hdr = rsdt->entry[i++] + HHDM_OFFSET)) {
        if (hdr->signature == signature)
            return (void*)HHDM_OFFSET + rsdt->entry[--i];
    }
}



void acpi(uint32_t **lapic, uint8_t *NMI) {
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

    *lapic = HHDM_OFFSET + madt->lapic;
    
    return;
}



