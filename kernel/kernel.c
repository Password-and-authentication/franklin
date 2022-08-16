#include <stdint.h>
#include <stddef.h>
#include "limine.h"
#include "franklin/defs.h"
#include "franklin/mmu.h"
#include "franklin/cpu.h"
#include "franklin/idt.h"
#include "franklin/69.h"
#include "franklin/spinlock.h"
#include "franklin/apic.h"


int x = 1;
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

extern void isr_kbd(void);

void kmain(void) {
    init_idt();
    struct limine_memmap_response *memmap = memmap_request.response;
    initbmap(memmap);

    int i = 0;
    while (isfree(i++));

    init_lock(&spinlock);
    init_vmm();
    init_acpi();
    MADT *madt = get_acpi_sdt(MADT_C);
    walk_madt(madt);
    pic_remap(0x20);
    new_irq(33, isr_kbd);
    unmask_irq(0);
    init_apic(madt->lapic + HHDM_OFFSET);

    init_cpu();
    init_kbd();
    unmask_irq(1);

    while (1) {
      /* print("hello"); */
      sleep(1000);
    }
		  



    for(;;)
        asm ("hlt");
}


