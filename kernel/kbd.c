#include <stdint.h>
#include "../ACPI/acpi.h"
#include "../69.h"
#include "kbd.h"







void init_kbd() {
    
    if (check_ps2() == 0)
        return;
    
    char buffer;
    asm ("movb $0x20, %al; outb %al, $0x64");
    asm volatile("inb $0x64, %%al; mov %%al, %0" : "=r" (buffer) :: "rax");
    asm volatile("inb $0x60, %%al; mov %%al, %0" : "=r" (buffer) :: "rax");




}


int check_ps2() {
    FADT *fadt = get_acpi_sdt(FADT_CODE);

    return (fadt->IAPC_BOOT_ARCH & (1 << 1));
}


