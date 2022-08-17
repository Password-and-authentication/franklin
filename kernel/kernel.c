#include "limine.h"
#include "franklin/defs.h"
#include "franklin/mmu.h"
#include "franklin/cpu.h"
#include "franklin/idt.h"
#include "franklin/69.h"
#include "franklin/spinlock.h"
#include "franklin/apic.h"
#include "franklin/kbd.h"
#include "franklin/pic.h"
#include "franklin/acpi.h"
#include "franklin/interrupt.h"



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
    terminal_res->write(terminal, s, strlen(s));
    release(&spinlock);
}


void kmain(void) {
    struct limine_memmap_response *memmap = memmap_request.response;
    initbmap(memmap);

    init_lock(&spinlock);
    init_vmm();
    init_acpi(); // set global variable RSDT
    MADT *madt = get_acpi_sdt(MADT_C);
    walk_madt(madt); // get info about MADT table
    init_interrupt();
    init_pit(1000); // 1000 hz
    
    // sets LAPIC registers and starts the LAPIC timer (the first CPU will also configure it)
    init_apic((unsigned int*)((unsigned long)madt->lapic + HHDM_OFFSET)); 
    init_cpu(); // init 2nd CPU, (init_apic() gets called here aswell)

    for(;;)
        asm ("hlt");
}


