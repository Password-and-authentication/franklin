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
    // init_apic(lapic, *NMI);
    
    return;
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
    lapicc = lapic;
}


void init_timer(uint32_t* lapic) {
    *(uint32_t*)((uint64_t)lapic + TIMER_REG) = 1 << 17 | 32;
    *(uint32_t*)((uint64_t)lapic + DIVIDE_REG) = 0x3;
    *(uint32_t*)((uint64_t)lapic + INITCOUNT) = 10000;
}
