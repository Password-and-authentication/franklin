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
#include "kbd.h"
#include "pic.h"
#include "io.h"



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
    acquire(&spinlock);
    terminal_res->write(terminal, s, 1);
    release(&spinlock);
}



void kmain(void) {
    init_idt();
    struct limine_memmap_response *memmap = memmap_request.response;
    // initbmap(memmap);

    // int i = 0;
    // while (isfree(i++));

    // init_vmm();    
    init_acpi();
    // MADT *madt = get_acpi_sdt(MADT_C);
    // walk_madt(madt);
    // init_apic(madt->lapic + HHDM_OFFSET);

    // init_cpu();    
    init_kbd();
    pic_remap(0x20);
    unmask_irq(1);


    for(;;)
        asm ("hlt");
}


