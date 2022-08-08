#include <stdint.h>
#include <stddef.h>
#include "limine.h"
#include "defs.h"
#include "idt.h"
#include "mmu.h"
#include "cpu.h"
#include "../69.h"
#include "spinlock.h"
#include "apic.h"
#include "../ACPI/acpi.h"



static volatile struct limine_terminal_request terminal_request = {
    .id = LIMINE_TERMINAL_REQUEST,
    .revision = 0
};

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0,
};


void print(void* s) {
    struct limine_terminal_response *terminal_res = terminal_request.response;
    struct limine_terminal *terminal = terminal_res->terminals[0];
    terminal_res->write(terminal, s, strlen(s));
}

extern uint64_t *bitmap;



void lmao() {
    acquire(&spinlock);

    int x = 10;
    int y = 20;
    release(&spinlock);
    return;
}

#define L 0x43495041


void kmain(void) {
    init_idt();
    struct limine_memmap_response *memmap = memmap_request.response;
    initbmap(memmap);

    
    init_acpi();
    MADT *madt = get_acpi_sdt(0x43495041);
    uint8_t NMI;
    walk_madt(madt, &NMI);
    init_apic(madt->lapic + HHDM_OFFSET, NMI);


    // init_cpu();    
    init_lock(&spinlock);


    lmao();

    for(;;)
        asm ("hlt");
}


